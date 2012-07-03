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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

// C
#if HAVE_ARCHIVE_H
#  include <archive.h>
#endif
#if HAVE_ARCHIVE_ENTRY_H
#  include <archive_entry.h>
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

#ifndef HAVE_STRNDUP
char*
strndup(const char* s, size_t n)
  {
    char* rv = static_cast<char*>(malloc(sizeof(char)*n+1));
    if (rv == NULL)
      {
          return rv;
      }
    rv = static_cast<char*>(memcpy(rv, s, n));
    if (rv == NULL)
      {
        return rv;
      }
    rv[n] = '\0';
    return rv;
  }
#endif

namespace hfst_ol 
  {

ZHfstOspellerXmlMetadata::ZHfstOspellerXmlMetadata()
  {
    info_.locale_ = "und";
  }

#if HAVE_LIBXML
void
ZHfstOspellerXmlMetadata::parse_xml(const xmlpp::Document* doc)
  {
    if (NULL == doc)
      {
        throw ZHfstMetaDataParsingError("Cannot parse XML data");
      }
    xmlpp::Node* cur = doc->get_root_node();
    // check validity
    if (NULL == cur)
      {
        throw ZHfstMetaDataParsingError("cannot parse XML file");
      }
    xmlpp::Element* el = dynamic_cast<xmlpp::Element*>(cur);
    if (NULL == el)
      {
        throw ZHfstMetaDataParsingError("cannot parse XML file");
      }
    else if (el->get_name() != "hfstspeller")
      {
        throw ZHfstMetaDataParsingError("could not find <hfstspeller> "
                                        "root from XML file");
      }
    // check versions
    const xmlpp::Attribute* hfstversion = el->get_attribute("hfstversion");
    if (NULL == hfstversion)
      {
        throw ZHfstMetaDataParsingError("No hfstversion attribute in root");
      }
    else if (hfstversion->get_value() != "3")
      {
        throw ZHfstMetaDataParsingError("Unrecognised HFST version...");
      }
    const xmlpp::Attribute* dtdversion = el->get_attribute("dtdversion");
    if (NULL == dtdversion)
      {
        throw ZHfstMetaDataParsingError("No dtdversion attribute in root");
      }
    else if (dtdversion->get_value() != "1.0")
      {
        throw ZHfstMetaDataParsingError("Unrecognised DTD version...");
      }
    // parse 
    xmlpp::Node::NodeList nodes = cur->get_children();
    for (xmlpp::Node::NodeList::iterator node = nodes.begin();
         node != nodes.end();
         ++node)
      {
        el = dynamic_cast<xmlpp::Element*>(*node);
        if ((*node)->get_name() == "info")
          {
            xmlpp::Node::NodeList infos = (*node)->get_children();
            for (xmlpp::Node::NodeList::iterator info = infos.begin();
                 info != infos.end();
                 ++info)
              {
                el = dynamic_cast<xmlpp::Element*>(*info);
                if ((*info)->get_name() == "locale")
                  {
                    if ((info_.locale_ != "und") && 
                        (info_.locale_ != el->get_child_text()->get_content()))
                      {
                        // locale in XML mismatches previous definition
                        // warnable, but overridden as per spec.
                        fprintf(stderr, "Warning: mismatched languages in "
                                "file data (%s) and XML (%s)\n",
                                info_.locale_.c_str(), el->get_child_text()->get_content().c_str());
                      }
                    info_.locale_ = el->get_child_text()->get_content();
                  }
                else if ((*info)->get_name() == "title")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        info_.title_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        info_.title_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if ((*info)->get_name() == "description")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        info_.description_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        info_.description_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if ((*info)->get_name() == "version")
                  {
                    const xmlpp::Attribute* revision = el->get_attribute("vcsrev");
                    if (revision != NULL)
                      {
                        info_.vcsrev_ = revision->get_value();
                      }
                    info_.version_ = el->get_child_text()->get_content();
                  }
                else if ((*info)->get_name() == "date")
                  {
                    info_.date_ = el->get_child_text()->get_content();
                  }
                else if ((*info)->get_name() == "producer")
                  {
                    info_.producer_ = el->get_child_text()->get_content();
                  }
                else if ((*info)->get_name() == "contact")
                  {
                    const xmlpp::Attribute* email = el->get_attribute("email");
                    const xmlpp::Attribute* website = el->get_attribute("website");
                    if (email != NULL)
                      {
                        info_.email_ = email->get_value();
                      }
                    if (website != NULL)
                      {
                        info_.website_ = website->get_value();
                      }
                  }
                else
                  {
                    const xmlpp::TextNode* text = dynamic_cast<xmlpp::TextNode*>(*info);
                    if ((text == NULL) || (!text->is_white_space()))
                      {
                        fprintf(stderr, "DEBUG: unknown info node %s\n",
                                (*info)->get_name().c_str());
                      }
                  }
              } // while info childs
          } // if info node
        else if ((*node)->get_name() == "acceptor")
          {
            xmlpp::Attribute* xid = el->get_attribute("id");
            if (xid == NULL)
              {
                throw ZHfstMetaDataParsingError("id missing in acceptor");
              }
            const char* id = xid->get_value().c_str();
            const char* p = strchr(id, '.');
            if (p == NULL)
              {
                throw ZHfstMetaDataParsingError("illegal id in acceptor");
              }
            size_t descr_len = 0;
            for (const char* q = p + 1; q != '\0'; q++)
              {
                if (*q == '.')
                  {
                    break;
                  }
                descr_len++;
              }
            char* descr = strndup(p + 1, descr_len);
            acceptor_[descr].descr_ = descr;
            acceptor_[descr].id_ = id;
            const xmlpp::Attribute* trtype = el->get_attribute("transtype");
            if (trtype != NULL)
              {
                acceptor_[descr].transtype_ = trtype->get_value();
              }
            const xmlpp::Attribute* xtype = el->get_attribute("type");
            if (xtype != NULL)
              {
                acceptor_[descr].type_ = xtype->get_value();
              }
            xmlpp::Node::NodeList accs = (*node)->get_children();
            for (xmlpp::Node::NodeList::iterator acc = accs.begin();
                 acc != accs.end();
                 ++acc)
              {
                el = dynamic_cast<xmlpp::Element*>(*acc);
                if ((*acc)->get_name() == "title")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        acceptor_[descr].title_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        acceptor_[descr].title_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if ((*acc)->get_name() == "description")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        acceptor_[descr].description_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        acceptor_[descr].description_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if (el != NULL)
                  {
                    fprintf(stderr, "DEBUG: unknown info node %s\n",
                            el->get_name().c_str());
                  }
              }
          } // acceptor node
        else if ((*node)->get_name() == "errmodel")
          {
            xmlpp::Attribute* xid = el->get_attribute("id");
            if (xid == NULL)
              {
                throw ZHfstMetaDataParsingError("id missing in errmodel");
              }
            const char* id = (xid->get_value().c_str());
            const char* p = strchr(id, '.');
            if (p == NULL)
              {
                throw ZHfstMetaDataParsingError("illegal id in errmodel");
              }
            size_t descr_len = 0;
            for (const char* q = p + 1; q != '\0'; q++)
              {
                if (*q == '.')
                  {
                    break;
                  }
                descr_len++;
              }
            char* descr = strndup(p + 1, descr_len);
            errmodel_.push_back(ZHfstOspellerErrModelMetadata());
            if (descr != NULL)
              {
            errmodel_[0].descr_ = descr;
              }
            errmodel_[0].id_ = id;
            xmlpp::Node::NodeList errms = (*node)->get_children();
            for (xmlpp::Node::NodeList::iterator errm = errms.begin();
                   errm != errms.end();
                   ++errm)
              {
                el = dynamic_cast<xmlpp::Element*>(*errm);
                if ((*errm)->get_name() == "title")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        errmodel_[0].title_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        errmodel_[0].title_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if ((*errm)->get_name() == "description")
                  {
                    const xmlpp::Attribute* lang = el->get_attribute("lang");
                    if (lang != NULL)
                      {
                        errmodel_[0].description_[lang->get_value()] = el->get_child_text()->get_content();
                      }
                    else
                      {
                        errmodel_[0].description_[info_.locale_] = el->get_child_text()->get_content();
                      }
                  }
                else if ((*errm)->get_name() == "type")
                  {
                    const xmlpp::Attribute* xtype = el->get_attribute("type");
                    if (xtype != NULL)
                      {
                        errmodel_[0].type_.push_back(xtype->get_value());

                      }
                    else
                      {
                        throw ZHfstMetaDataParsingError("No type in type");
                      }
                  }
                else if ((*errm)->get_name() == "model")
                  {
                    errmodel_[0].model_.push_back(el->get_child_text()->get_content());
                  }
                else if (el != NULL)
                  {
                    fprintf(stderr, "DEBUG: unknown errm node %s\n",
                            (*errm)->get_name().c_str());
                  }
              }
          } // errmodel node
        else if (el != NULL)
          {
            fprintf(stderr, "DEBUG: unknown top level node %s\n",
                    el->get_name().c_str());
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
    ZHfstOspellerXmlMetadata::read_xml(const void*, size_t)
      {}
void
    ZHfstOspellerXmlMetadata::read_xml(const char*)
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

#if ZHFST_EXTRACT_TO_MEM
static
void*
extract_to_mem(archive* ar, archive_entry* entry, size_t* n)
  {
    size_t full_length = 0;
    const struct stat* st = archive_entry_stat(entry);
    size_t buffsize = st->st_size;
    void* buff = malloc(sizeof(char) * buffsize);
    for (;;)
      {
        size_t curr = archive_read_data(ar, buff, buffsize);
        if (0 == curr)
          {
            break;
          }
        else if (buffsize <= curr)
          {
            fprintf(stderr, "DEBUG: curr: %zu, buffsize: %zu\n",
                    curr, buffsize);
            full_length += curr;
          } 
        else if (curr < buffsize)
          {
            fprintf(stderr, "DEBUG: curr: %zu, buffsize: %zu\n",
                    curr, buffsize);
          }
        else
          {
            throw ZHfstZipReadingError("Archive broken...");
          }
      }
    *n = full_length;
    return buff;
  }
#endif

#if ZHFST_EXTRACT_TO_TMP_DIR
static
char*
extract_to_tmp_dir(archive* ar)
  {
    char* rv = strdup("/tmp/zhfstospellXXXXXXXX");
    int temp_fd = mkstemp(rv);
    int rr = archive_read_data_into_fd(ar, temp_fd);
    if (rr == ARCHIVE_EOF)
      {
        rr = ARCHIVE_OK;
      }
    else 
      {
        throw ZHfstZipReadingError("Archive not EOF'd");
      }
    close(temp_fd);
    return rv;
  }
#endif

ZHfstOspeller::ZHfstOspeller() :
    can_spell_(false),
    can_correct_(false),
    current_speller_(0),
    current_sugger_(0)
    {
    }

ZHfstOspeller::~ZHfstOspeller()
  {
    if ((current_speller_ != NULL) && (current_sugger_ != NULL))
      {
        if (current_speller_ != current_sugger_)
          {
            delete current_speller_;
            delete current_sugger_;
          }
        else
          {
            delete current_speller_;
          }
        current_sugger_ = 0;
        current_speller_ = 0;
      }
    can_spell_ = false;
    can_correct_ = false;
  }

bool
ZHfstOspeller::spell(const string& wordform)
  {
    if (can_spell_ && (current_speller_ != 0))
      {
        char* wf = strdup(wordform.c_str());
        bool rv = current_speller_->check(wf);
        free(wf);
        return rv;
      }
    return false;
  }

CorrectionQueue
ZHfstOspeller::suggest(const string& wordform)
  {
    CorrectionQueue rv;
    if ((can_correct_) && (current_sugger_ != 0))
      {
        char* wf = strdup(wordform.c_str());
        rv = current_sugger_->correct(wf);
        free(wf);
        return rv;
      }
    return rv;
  }

void
ZHfstOspeller::read_zhfst(const string& filename)
  {
    struct archive* ar = archive_read_new();
#if ZHFST_EXTRACT_TO_TMP_DIR
    struct archive* aw = archive_write_disk_new();
#endif
    struct archive_entry* entry = 0;
    archive_read_support_compression_all(ar);
    archive_read_support_format_all(ar);
#if ZHFST_EXTRACT_TO_TMP_DIR
    archive_write_disk_set_options(aw, 
                                   ZHFST_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(aw);
#endif
    int rr = archive_read_open_filename(ar, filename.c_str(), 10240);
    if (rr != ARCHIVE_OK)
      {
        throw ZHfstZipReadingError("Archive not OK");
      }
    for (int rr = archive_read_next_header(ar, &entry); 
         rr != ARCHIVE_EOF;
         rr = archive_read_next_header(ar, &entry))
      {
        if (rr != ARCHIVE_OK)
          {
            throw ZHfstZipReadingError("Archive not OK");
          }
        char* filename = strdup(archive_entry_pathname(entry));
        if (strncmp(filename, "acceptor.", strlen("acceptor.")) == 0)
          {
#if ZHFST_EXTRACT_TO_TMP_DIR
            char* temporary = extract_to_tmp_dir(ar);
#elif ZHFST_EXTRACT_TO_MEM
            size_t total_length = 0;
            void* full_data = extract_to_mem(ar, entry, &total_length);
#endif
            char* p = filename;
            p += strlen("acceptor.");
            size_t descr_len = 0;
            for (const char* q = p; *q != '\0'; q++)
              {
                if (*q == '.')
                  {
                    break;
                  }
                else
                    {
                      descr_len++;
                    }
              }
            char* descr = strndup(p, descr_len);
#if ZHFST_EXTRACT_TO_TMP_DIR
            FILE* f = fopen(temporary, "r");
            if (f == NULL)
              {
                  throw ZHfstTemporaryWritingError("reading acceptor back "
                                                   "from temp file");
              }
            Transducer* trans = new Transducer(f);
#elif ZHFST_EXTRACT_TO_MEM
            Transducer* trans = new Transducer(reinterpret_cast<char*>(full_data));
#endif
            acceptors_[descr] = trans;
          }
        else if (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0)
          {
#if ZHFST_EXTRACT_TO_TMP_DIR
            char* temporary = extract_to_tmp_dir(ar);
#elif ZHFST_EXTRACT_TO_MEM
            size_t total_length = 0;
            void* full_data = extract_to_mem(ar, entry, &total_length);
#endif
            const char* p = filename;
            p += strlen("errmodel.");
            size_t descr_len = 0;
            for (const char* q = p; *q != '\0'; q++)
              {
                if (*q == '.')
                  {
                    break;
                  }
                else
                  {
                    descr_len++;
                  }
              }
            char* descr = strndup(p, descr_len);
#if ZHFST_EXTRACT_TO_TMP_DIR
            FILE* f = fopen(temporary, "r");
            if (NULL == f)
              {
                throw ZHfstTemporaryWritingError("reading errmodel back "
                                                 "from temp file");
              }
            Transducer* trans = new Transducer(f);
#elif ZHFST_EXTRACT_TO_MEM
            Transducer* trans = new Transducer(reinterpret_cast<char*>(full_data));
#endif
            errmodels_[descr] = trans;
          } // if acceptor or errmodel
        else if (strcmp(filename, "index.xml") == 0)
          {
#if ZHFST_EXTRACT_TO_TMP_DIR
            char* temporary = extract_to_tmp_dir(ar);
            metadata_.read_xml(temporary);
#elif ZHFST_EXTRACT_TO_MEM
            size_t xml_len = 0;
            void* full_data = extract_to_mem(ar, entry, &xml_len);
            metadata_.read_xml(reinterpret_cast<char*>(full_data), xml_len);
#endif

          }
        else
          {
            fprintf(stderr, "Unknown file in archive %s\n", filename);
          }
        free(filename);
      } // while r != ARCHIVE_EOF
    archive_read_close(ar);
    if ((errmodels_.find("default") != errmodels_.end()) &&
        (acceptors_.find("default") != acceptors_.end()))
      {
        current_speller_ = new Speller(
                                       errmodels_["default"],
                                       acceptors_["default"]
                                       );
        current_sugger_ = current_speller_;
        can_spell_ = true;
        can_correct_ = true;
      }
    else if ((acceptors_.size() > 0) && (errmodels_.size() > 0))
      {
        fprintf(stderr, "Could not find default speller, using %s %s\n",
                acceptors_.begin()->first.c_str(),
                errmodels_.begin()->first.c_str());
        current_speller_ = new Speller(
                                       errmodels_.begin()->second,
                                       acceptors_.begin()->second
                                       );
        current_sugger_ = current_speller_;
        can_spell_ = true;
        can_correct_ = true;
      }
    else if ((acceptors_.size() > 0) && 
             (acceptors_.find("default") != acceptors_.end()))
      {
        current_speller_ = new Speller(0, acceptors_["default"]);
        current_sugger_ = current_speller_;
        can_spell_ = true;
        can_correct_ = false;
      }
    else if (acceptors_.size() > 0)
      {
        current_speller_ = new Speller(0, acceptors_.begin()->second);
        current_sugger_ = current_speller_;
        can_spell_ = true;
        can_correct_ = false;
      }
  }

void
ZHfstOspeller::read_legacy(const std::string& path)
  {
    string spellerfile = path + "/spl.hfstol";
    string suggerfile = path + "/sug.hfstol";
    string errorfile = path + "/err.hfstol"; 
    bool speller = false;
    bool sugger = false;
    bool error = false;
    FILE* f = fopen(spellerfile.c_str(), "r");
    if (f != NULL)
      {
        Transducer* trans = new Transducer(f);
        acceptors_["default"] = trans;
        speller = true;
      }
    fclose(f);
    f = fopen(suggerfile.c_str(), "r");
    if (f != NULL)
      {
        Transducer* trans = new Transducer(f);
        acceptors_["suggestion"] = trans;
        sugger = true;
      }
    fclose(f);
    f = fopen(errorfile.c_str(), "r");
    if (f != NULL)
      {
        Transducer* trans = new Transducer(f);
        errmodels_["default"] = trans;
        error = true;
      }
    if (speller && sugger && error)
      {
        current_speller_ = new Speller(acceptors_["default"],
                                       errmodels_["default"]);
        current_sugger_ = new Speller(acceptors_["suggestion"],
                                      errmodels_["default"]);
      }
    else if (speller && error)
      {
        current_speller_ = new Speller(acceptors_["default"],
                                       errmodels_["default"]);
        current_sugger_ = current_speller_;
      }
    else
      {
        throw ZHfstLegacyReadingError("no readable legacy automata found");
      }
  }
 
string
ZHfstOspeller::metadata_dump() const
  {
    return metadata_.debug_dump();

  }

ZHfstException::ZHfstException() :
    what_("unknown")
    {}
ZHfstException::ZHfstException(const std::string& message) :
    what_(message)
      {}

            
const char* 
ZHfstException::what()
  {
    return what_.c_str();
  }

            
ZHfstMetaDataParsingError::ZHfstMetaDataParsingError(const std::string& message):
    ZHfstException(message)
  {}
ZHfstXmlParsingError::ZHfstXmlParsingError(const std::string& message):
    ZHfstException(message)
  {}
ZHfstZipReadingError::ZHfstZipReadingError(const std::string& message):
    ZHfstException(message)
  {}
ZHfstTemporaryWritingError::ZHfstTemporaryWritingError(const std::string& message):
    ZHfstException(message)
    {}
ZHfstLegacyReadingError::ZHfstLegacyReadingError(const std::string& message):
    ZHfstException(message)
    {}
 
} // namespace hfst_ol
    

