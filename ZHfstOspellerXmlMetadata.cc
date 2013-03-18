/* -*- Mode: C++ -*- */
// Copyright 2010 University of Helsinki
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

//! XXX: Valgrind note: all errors about invalid reads in wcscmp are because of:
//! https://bugzilla.redhat.com/show_bug.cgi?id=755242

#if HAVE_CONFIG_H
#  include <config.h>
#endif

// C++
#if HAVE_LIBXML
#  include <libxml++/libxml++.h>
#endif
#include <string>
#include <map>

using std::string;
using std::map;

// local
#include "ospell.h"
#include "hfst-ol.h"
#include "ZHfstOspeller.h"
#include "ZHfstOspellerXmlMetadata.h"

namespace hfst_ol 
  {

ZHfstOspellerXmlMetadata::ZHfstOspellerXmlMetadata()
  {
    info_.locale_ = "und";
  }

#if HAVE_LIBXML
void
ZHfstOspellerXmlMetadata::verify_hfstspeller(xmlpp::Node* rootNode)
  {
    xmlpp::Element* rootElement = dynamic_cast<xmlpp::Element*>(rootNode);
    if (NULL == rootElement)
      {
        throw ZHfstMetaDataParsingError("Root node is not an element");
      }
    const Glib::ustring rootName = rootElement->get_name();
    if (rootName != "hfstspeller")
      {
        throw ZHfstMetaDataParsingError("could not find <hfstspeller> "
                                        "root from XML file");
      }
    // check versions
    const xmlpp::Attribute* hfstversion = 
        rootElement->get_attribute("hfstversion");
    if (NULL == hfstversion)
      {
        throw ZHfstMetaDataParsingError("No hfstversion attribute in root");
      }
    const Glib::ustring hfstversionValue = hfstversion->get_value();
    if (hfstversionValue != "3")
      {
        throw ZHfstMetaDataParsingError("Unrecognised HFST version...");
      }
    const xmlpp::Attribute* dtdversion = rootElement->get_attribute("dtdversion");
    if (NULL == dtdversion)
      {
        throw ZHfstMetaDataParsingError("No dtdversion attribute in root");
      }
    const Glib::ustring dtdversionValue = dtdversion->get_value();
    if (dtdversionValue != "1.0")
      {
        throw ZHfstMetaDataParsingError("Unrecognised DTD version...");
      }
  }

void
ZHfstOspellerXmlMetadata::parse_info(xmlpp::Node* infoNode)
  {
    xmlpp::Node::NodeList infos = infoNode->get_children();
    for (xmlpp::Node::NodeList::iterator info = infos.begin();
         info != infos.end();
         ++info)
      {
        const Glib::ustring infoName = (*info)->get_name();
        if (infoName == "locale")
          {
            parse_locale(*info);
          }
        else if (infoName == "title")
          {
            parse_title(*info);
          }
        else if (infoName == "description")
          {
            parse_description(*info);
          }
        else if (infoName == "version")
          {
            parse_version(*info);
          }
        else if (infoName == "date")
          {
            parse_date(*info);
          }
        else if (infoName == "producer")
          {
            parse_producer(*info);
          }
        else if (infoName == "contact")
          {
            parse_contact(*info);
          }
        else
          {
            const xmlpp::TextNode* text = dynamic_cast<xmlpp::TextNode*>(*info);
            if ((text == NULL) || (!text->is_white_space()))
              {
                fprintf(stderr, "DEBUG: unknown info node %s\n",
                        infoName.c_str());
              }
          }
      } // while info childs
  }

void
ZHfstOspellerXmlMetadata::parse_locale(xmlpp::Node* localeNode)
  {
    xmlpp::Element* localeElement = dynamic_cast<xmlpp::Element*>(localeNode);
    const Glib::ustring localeContent = localeElement->get_child_text()->get_content();
    if ((info_.locale_ != "und") && (info_.locale_ != localeContent))
      {
        // locale in XML mismatches previous definition
        // warnable, but overridden as per spec.
        fprintf(stderr, "Warning: mismatched languages in "
                "file data (%s) and XML (%s)\n",
                info_.locale_.c_str(), localeContent.c_str());
      }
    info_.locale_ = localeContent;
  }

void
ZHfstOspellerXmlMetadata::parse_title(xmlpp::Node* titleNode)
  {
    xmlpp::Element* titleElement = dynamic_cast<xmlpp::Element*>(titleNode);
    const xmlpp::Attribute* lang = titleElement->get_attribute("lang");
    if (lang != NULL)
      {
        info_.title_[lang->get_value()] = titleElement->get_child_text()->get_content();
      }
    else
      {
        info_.title_[info_.locale_] = titleElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_description(xmlpp::Node* descriptionNode)
  {
    xmlpp::Element* descriptionElement = 
        dynamic_cast<xmlpp::Element*>(descriptionNode);
    const xmlpp::Attribute* lang = descriptionElement->get_attribute("lang");
    if (lang != NULL)
      {
        info_.description_[lang->get_value()] = descriptionElement->get_child_text()->get_content();
      }
    else
      {
        info_.description_[info_.locale_] = descriptionElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_version(xmlpp::Node* versionNode)
  {
    xmlpp::Element* versionElement = dynamic_cast<xmlpp::Element*>(versionNode);
    const xmlpp::Attribute* revision = versionElement->get_attribute("vcsrev");
    if (revision != NULL)
      {
        info_.vcsrev_ = revision->get_value();
      }
    info_.version_ = versionElement->get_child_text()->get_content();
  }

void
ZHfstOspellerXmlMetadata::parse_date(xmlpp::Node* dateNode)
  {
    xmlpp::Element* dateElement = 
        dynamic_cast<xmlpp::Element*>(dateNode);
    info_.date_ = dateElement->get_child_text()->get_content();
  }

void
ZHfstOspellerXmlMetadata::parse_producer(xmlpp::Node* producerNode)
  {
    xmlpp::Element* producerElement = 
        dynamic_cast<xmlpp::Element*>(producerNode);
    info_.producer_ = producerElement->get_child_text()->get_content();
  }

void
ZHfstOspellerXmlMetadata::parse_contact(xmlpp::Node* contactNode)
  {
    xmlpp::Element* contactElement = dynamic_cast<xmlpp::Element*>(contactNode);
    const xmlpp::Attribute* email = contactElement->get_attribute("email");
    const xmlpp::Attribute* website = contactElement->get_attribute("website");
    if (email != NULL)
      {
        info_.email_ = email->get_value();
      }
    if (website != NULL)
      {
        info_.website_ = website->get_value();
      }
  }

static
bool
validate_automaton_id(const Glib::ustring idstring)
  {
    const char* id = idstring.c_str();
    const char* p = strchr(id, '.');
    if (p == NULL)
      {
        return false;
      }
    const char* q = strchr(p + 1, '.');
    if (q == NULL)
      {
        return false;
      }
    return true;
  }

static
char*
get_automaton_descr_from_id(const Glib::ustring idstring)
  {
    const char* id = idstring.c_str();
    const char* p = strchr(id, '.');
    const char* q = strchr(p + 1, '.');
    return strndup(p + 1, q - p);
  }

void
ZHfstOspellerXmlMetadata::parse_acceptor(xmlpp::Node* acceptorNode)
  {
    xmlpp::Element* acceptorElement = 
        dynamic_cast<xmlpp::Element*>(acceptorNode);
    xmlpp::Attribute* xid = acceptorElement->get_attribute("id");
    if (xid == NULL)
      {
        throw ZHfstMetaDataParsingError("id missing in acceptor");
      }
    const Glib::ustring xidValue = xid->get_value();
    if (validate_automaton_id(xidValue) == false)
      {
        throw ZHfstMetaDataParsingError("Invalid id in acceptor");
      }
    char* descr = get_automaton_descr_from_id(xidValue);
    acceptor_[descr].descr_ = descr;
    acceptor_[descr].id_ = xidValue;
    const xmlpp::Attribute* trtype = 
        acceptorElement->get_attribute("transtype");
    if (trtype != NULL)
      {
        acceptor_[descr].transtype_ = trtype->get_value();
      }
    const xmlpp::Attribute* xtype = acceptorElement->get_attribute("type");
    if (xtype != NULL)
      {
        acceptor_[descr].type_ = xtype->get_value();
      }
    xmlpp::Node::NodeList accs = acceptorNode->get_children();
    for (xmlpp::Node::NodeList::iterator acc = accs.begin();
         acc != accs.end();
         ++acc)
      {
        const Glib::ustring accName = (*acc)->get_name();
        if (accName == "title")
          {
            parse_title(*acc, descr);
          }
        else if (accName == "description")
          {
            parse_description(*acc, descr);
          }
        else
          {
            const xmlpp::TextNode* text = dynamic_cast<xmlpp::TextNode*>(*acc);
            if ((text == NULL) || (!text->is_white_space()))
              {
                fprintf(stderr, "DEBUG: unknown acceptor node %s\n",
                        accName.c_str());
              }
          }
      }
    free(descr);
  }

void
ZHfstOspellerXmlMetadata::parse_title(xmlpp::Node* titleNode, 
                                      const string& descr)
  {
    xmlpp::Element* titleElement = dynamic_cast<xmlpp::Element*>(titleNode);
    const xmlpp::Attribute* lang = titleElement->get_attribute("lang");
    if (lang != NULL)
      {
        acceptor_[descr].title_[lang->get_value()] = titleElement->get_child_text()->get_content();
      }
    else
      {
        acceptor_[descr].title_[info_.locale_] = titleElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_description(xmlpp::Node* descriptionNode,
                                            const string& descr)
  {
    xmlpp::Element* descriptionElement = 
        dynamic_cast<xmlpp::Element*>(descriptionNode);
    const xmlpp::Attribute* lang = descriptionElement->get_attribute("lang");
    if (lang != NULL)
      {
        acceptor_[descr].description_[lang->get_value()] = descriptionElement->get_child_text()->get_content();
      }
    else
      {
        acceptor_[descr].description_[info_.locale_] = descriptionElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_errmodel(xmlpp::Node* errmodelNode)
  {
    xmlpp::Element* errmodelElement = 
        dynamic_cast<xmlpp::Element*>(errmodelNode);
    xmlpp::Attribute* xid = errmodelElement->get_attribute("id");
    if (xid == NULL)
      {
        throw ZHfstMetaDataParsingError("id missing in errmodel");
      }
    const Glib::ustring xidValue = xid->get_value();
    if (validate_automaton_id(xidValue) == false)
      {
        throw ZHfstMetaDataParsingError("Invalid id in errmodel");
      }
    char* descr = get_automaton_descr_from_id(xidValue);
    errmodel_.push_back(ZHfstOspellerErrModelMetadata());
    size_t errm_count = errmodel_.size() - 1;
    if (descr != NULL)
      {
        errmodel_[errm_count].descr_ = descr;
      }
    free(descr);
    errmodel_[errm_count].id_ = xidValue;
    xmlpp::Node::NodeList errms = errmodelNode->get_children();
    for (xmlpp::Node::NodeList::iterator errm = errms.begin();
           errm != errms.end();
           ++errm)
      {
        const Glib::ustring errmName = (*errm)->get_name();
        if (errmName == "title")
          {
            parse_title(*errm, errm_count);
          }
        else if (errmName == "description")
          {
            parse_description(*errm, errm_count);
          }
        else if (errmName == "type")
          {
            parse_type(*errm, errm_count);
          }
        else if (errmName == "model")
          {
            parse_model(*errm, errm_count);
          }
        else
          {
            const xmlpp::TextNode* text = dynamic_cast<xmlpp::TextNode*>(*errm);
            if ((text == NULL) || (!text->is_white_space()))
              {
                fprintf(stderr, "DEBUG: unknown errmodel node %s\n",
                        errmName.c_str());
              }
          }
      }
  }

void
ZHfstOspellerXmlMetadata::parse_title(xmlpp::Node* titleNode, 
                                      size_t errm_count)
  {
    xmlpp::Element* titleElement = dynamic_cast<xmlpp::Element*>(titleNode);
    const xmlpp::Attribute* lang = titleElement->get_attribute("lang");
    if (lang != NULL)
      {
        errmodel_[errm_count].title_[lang->get_value()] = titleElement->get_child_text()->get_content();
      }
    else
      {
        errmodel_[errm_count].title_[info_.locale_] = titleElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_description(xmlpp::Node* descriptionNode,
                                            size_t errm_count)
  {
    xmlpp::Element* descriptionElement = 
        dynamic_cast<xmlpp::Element*>(descriptionNode);
    const xmlpp::Attribute* lang = descriptionElement->get_attribute("lang");
    if (lang != NULL)
      {
        errmodel_[errm_count].description_[lang->get_value()] = descriptionElement->get_child_text()->get_content();
      }
    else
      {
        errmodel_[errm_count].description_[info_.locale_] = descriptionElement->get_child_text()->get_content();
      }
  }

void
ZHfstOspellerXmlMetadata::parse_type(xmlpp::Node* typeNode, size_t errm_count)
  {
    xmlpp::Element* typeElement = dynamic_cast<xmlpp::Element*>(typeNode);
    const xmlpp::Attribute* xtype = typeElement->get_attribute("type");
    if (xtype != NULL)
      {
        errmodel_[errm_count].type_.push_back(xtype->get_value());

      }
    else
      {
        throw ZHfstMetaDataParsingError("No type in type");
      }
  }

void
ZHfstOspellerXmlMetadata::parse_model(xmlpp::Node* modelNode, size_t errm_count)
  {
    xmlpp::Element* modelElement = dynamic_cast<xmlpp::Element*>(modelNode);
    errmodel_[errm_count].model_.push_back(modelElement->get_child_text()->get_content());
  }

void
ZHfstOspellerXmlMetadata::parse_xml(const xmlpp::Document* doc)
  {
    if (NULL == doc)
      {
        throw ZHfstMetaDataParsingError("Cannot parse XML data");
      }
    xmlpp::Node* rootNode = doc->get_root_node();
    // check validity
    if (NULL == rootNode)
      {
        throw ZHfstMetaDataParsingError("No root node in index XML");
      }
    verify_hfstspeller(rootNode);
    // parse 
    xmlpp::Node::NodeList nodes = rootNode->get_children();
    for (xmlpp::Node::NodeList::iterator node = nodes.begin();
         node != nodes.end();
         ++node)
      {
        const Glib::ustring nodename = (*node)->get_name();
        if (nodename == "info")
          {
            parse_info(*node);
          } // if info node
        else if (nodename == "acceptor")
          {
            parse_acceptor(*node);
          } // acceptor node
        else if (nodename == "errmodel")
          {
            parse_errmodel(*node);
          } // errmodel node
        else
          {
            const xmlpp::TextNode* text = dynamic_cast<xmlpp::TextNode*>(*node);
            if ((text == NULL) || (!text->is_white_space()))
              {
                fprintf(stderr, "DEBUG: unknown root node %s\n",
                        nodename.c_str());
              }
          } // unknown root child node
      }
  }

void
ZHfstOspellerXmlMetadata::read_xml(const char* xml_data, size_t xml_len)
  {
    xmlpp::DomParser parser;
    parser.set_substitute_entities();
    parser.parse_memory_raw(reinterpret_cast<const unsigned char*>(xml_data),
                            xml_len);
    this->parse_xml(parser.get_document());
  }

void
ZHfstOspellerXmlMetadata::read_xml(const string& filename)
  {
    xmlpp::DomParser parser;
    parser.set_substitute_entities();
    parser.parse_file(filename);
    this->parse_xml(parser.get_document());
  }
#else
void
    ZHfstOspellerXmlMetadata::read_xml(const char*, size_t)
      {}
void
    ZHfstOspellerXmlMetadata::read_xml(const string&)
      {}
#endif // HAVE_LIBXML


string
ZHfstOspellerXmlMetadata::debug_dump() const
  {
    string retval = "locale: " + info_.locale_ + "\n"
        "version: " + info_.version_ + " [vcsrev: " + info_.vcsrev_ + "]\n"
        "date: " + info_.date_ + "\n"
        "producer: " + info_.producer_ + "[email: <" + info_.email_ + ">, "
        "website: <" + info_.website_ + ">]\n";
    for (map<string,string>::const_iterator title = info_.title_.begin();
         title != info_.title_.end();
         ++title)
      {
        retval.append("title [" + title->first + "]: " + title->second + "\n");
      }
    for (map<string,string>::const_iterator description = info_.description_.begin();
         description != info_.description_.end();
         ++description)
      {
        retval.append("description [" + description->first + "]: " +
            description->second + "\n");
      }
    for (map<string,ZHfstOspellerAcceptorMetadata>::const_iterator acc = acceptor_.begin();
         acc != acceptor_.end();
         ++acc)
      {
        retval.append("acceptor[" + acc->second.descr_ + "] [id: " + acc->second.id_ +
                      ", type: " + acc->second.type_ + "trtype: " + acc->second.transtype_ +
                      "]\n");

        for (LanguageVersions::const_iterator title = acc->second.title_.begin();
             title != acc->second.title_.end();
             ++title)
          {
            retval.append("title [" + title->first + "]: " + title->second +
                          "\n");
          }
        for (LanguageVersions::const_iterator description = acc->second.description_.begin();
             description != acc->second.description_.end();
             ++description)
          {
            retval.append("description[" + description->first + "]: "
                          + description->second + "\n");
          }
      }
    for (std::vector<ZHfstOspellerErrModelMetadata>::const_iterator errm = errmodel_.begin();
         errm != errmodel_.end();
         ++errm)
      {
        retval.append("errmodel[" + errm->descr_ + "] [id: " + errm->id_ +
                      "]\n");

        for (LanguageVersions::const_iterator title = errm->title_.begin();
             title != errm->title_.end();
             ++title)
          {
            retval.append("title [" + title->first + "]: " + title->second +
                          "\n");
          }
        for (LanguageVersions::const_iterator description = errm->description_.begin();
             description != errm->description_.end();
             ++description)
          {
            retval.append("description[" + description->first + "]: "
                          + description->second + "\n");
          }
        for (std::vector<string>::const_iterator type = errm->type_.begin();
             type != errm->type_.end();
             ++type)
          {
            retval.append("type: " + *type + "\n");
          }
        for (std::vector<string>::const_iterator model = errm->model_.begin();
             model != errm->model_.end();
             ++model)
          {
            retval.append("model: " + *model + "\n");
          }
      }
    
    return retval;
  }

  } // namespace hfst_ol


