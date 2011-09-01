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
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <archive.h>
#include <archive_entry.h>
// C++
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

void
ZHfstOspellerXmlMetadata::read_xml(const string& filename)
  {
    xmlDocPtr doc;
    xmlNodePtr cur;
    doc = xmlParseFile(filename.c_str());
    cur = xmlDocGetRootElement(doc);
    // check validity
    if (NULL == cur)
      {
        xmlFreeDoc(doc);
        throw ZHfstMetaDataParsingError("cannot parse XML file");
      }
    if (xmlStrcmp(cur->name, xmlCharStrdup("hfstspeller")) != 0)
      {
        xmlFreeDoc(doc);
        throw ZHfstMetaDataParsingError("could not find <hfstspeller> "
                                        "from XML file");
      }
    // check versions
    // parse
    cur = cur->xmlChildrenNode;
    bool infoRead = false;
    while (cur != NULL)
      {
        if (xmlStrcmp(cur->name, xmlCharStrdup("info")) == 0)
          {
            xmlNodePtr info = cur->xmlChildrenNode;
            while (info != NULL)
              {
                xmlChar* data = xmlNodeListGetString(doc,
                                                      info->xmlChildrenNode,
                                                      1);
                if (xmlStrcmp(info->name, 
                              xmlCharStrdup("locale")) == 0)
                  {
                    if ((info_.locale_ != "und") && 
                        (xmlStrcmp(xmlCharStrdup(info_.locale_.c_str()), data) != 0))
                      {
                        // locale in XML mismatches previous definition
                        // warnable, but overridden as per spec.
                      }
                    info_.locale_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("title")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(info, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        info_.title_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        info_.title_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("description")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(info, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        info_.description_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        info_.description_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("version")) == 0)
                  {
                    xmlChar* revision = xmlGetProp(info, xmlCharStrdup("vcsrev"));
                    if (revision != NULL)
                      {
                        info_.vcsrev_ = reinterpret_cast<char*>(revision);
                      }
                    info_.version_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("date")) == 0)
                  {
                    info_.date_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("producer")) == 0)
                  {
                    info_.producer_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("contact")) == 0)
                  {
                    xmlChar* email = xmlGetProp(info, 
                                                xmlCharStrdup("email"));
                    xmlChar* website = xmlGetProp(info, 
                                                  xmlCharStrdup("website"));
                    if (email != NULL)
                      {
                        info_.email_ = reinterpret_cast<char*>(email);
                      }
                    if (website != NULL)
                      {
                        info_.website_ = reinterpret_cast<char*>(website);
                      }
                  }
                else
                  {
                    if (!xmlIsBlankNode(info))
                      {
                        fprintf(stderr, "DEBUG: unknown info node %s\n",
                                reinterpret_cast<const char*>(info->name));
                      }
                  }
                info = info->next;
              } // while info childs
          } // if info node
        else if (xmlStrcmp(cur->name,
                           xmlCharStrdup("acceptor")) == 0)
          {
            xmlChar* xid = xmlGetProp(cur, xmlCharStrdup("id"));
            if (xid == NULL)
              {
                throw ZHfstMetaDataParsingError("id missing in acceptor");
              }
            char* id = reinterpret_cast<char*>(xid);
            char* p = strchr(id, '.');
            if (p == NULL)
              {
                throw ZHfstMetaDataParsingError("illegal id in acceptor");
              }
            size_t descr_len = 0;
            for (char* q = p + 1; q != '\0'; q++)
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
            xmlChar* trtype = xmlGetProp(cur, xmlCharStrdup("transtype"));
            if (trtype != NULL)
              {
                acceptor_[descr].transtype_ = reinterpret_cast<char*>(trtype);
              }
            xmlChar* xtype = xmlGetProp(cur, xmlCharStrdup("type"));
            if (xtype != NULL)
              {
                acceptor_[descr].type_ = reinterpret_cast<char*>(xtype);
              }
            xmlNodePtr acc = cur->xmlChildrenNode;
            while (acc != NULL)
              {
                xmlChar* data = xmlNodeListGetString(doc,
                                                      acc->xmlChildrenNode,
                                                      1);
                if (xmlStrcmp(acc->name,
                                   xmlCharStrdup("title")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(acc, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        acceptor_[descr].title_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        acceptor_[descr].title_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(acc->name,
                                   xmlCharStrdup("description")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(acc, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        acceptor_[descr].description_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        acceptor_[descr].description_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else
                  {
                    if (!xmlIsBlankNode(acc))
                      {
                        fprintf(stderr, "DEBUG: unknown info node %s\n",
                                reinterpret_cast<const char*>(acc->name));
                      }
                  }
                acc = acc->next;
              }
          } // acceptor node
        else if (xmlStrcmp(cur->name,
                           xmlCharStrdup("errmodel")) == 0)
          {
            xmlChar* xid = xmlGetProp(cur, xmlCharStrdup("id"));
            if (xid == NULL)
              {
                throw ZHfstMetaDataParsingError("id missing in errmodel");
              }
            char* id = reinterpret_cast<char*>(xid);
            char* p = strchr(id, '.');
            if (p == NULL)
              {
                throw ZHfstMetaDataParsingError("illegal id in errmodel");
              }
            size_t descr_len = 0;
            for (char* q = p + 1; q != '\0'; q++)
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
            xmlNodePtr errm = cur->xmlChildrenNode;
            while (errm != NULL)
              {
                xmlChar* data = xmlNodeListGetString(doc,
                                                      errm->xmlChildrenNode,
                                                      1);
                if (xmlStrcmp(errm->name,
                                   xmlCharStrdup("title")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(errm, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        errmodel_[0].title_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        errmodel_[0].title_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(errm->name,
                                   xmlCharStrdup("description")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(errm, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        errmodel_[0].description_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        errmodel_[0].description_[info_.locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(errm->name,
                                   xmlCharStrdup("type")) == 0)
                  {
                    xmlChar* type = xmlGetProp(errm, xmlCharStrdup("type"));
                    if (type != NULL)
                      {
                        errmodel_[0].type_.push_back(reinterpret_cast<char*>(type));

                      }
                    else
                      {
                        throw ZHfstMetaDataParsingError("No type in type");
                      }
                  }
                else if (xmlStrcmp(errm->name,
                                   xmlCharStrdup("model")) == 0)
                  {
                    errmodel_[0].model_.push_back(reinterpret_cast<char*>(data));
                  }
                else
                  {
                    if (!xmlIsBlankNode(errm))
                      {
                        fprintf(stderr, "DEBUG: unknown info node %s\n",
                                reinterpret_cast<const char*>(errm->name));
                      }
                  }
                errm = errm->next;
              }
          } // errmodel node
        else
          {
            if (!xmlIsBlankNode(cur))
              {
                fprintf(stderr, "DEBUG: unknown top level node %s\n",
                        reinterpret_cast<const char*>(cur->name));
              }
          } // unknown root child node
        cur = cur->next;
      }
    xmlFreeDoc(doc);
  }

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

ZHfstOspeller::ZHfstOspeller() :
    current_speller_(0),
    current_sugger_(0),
    can_spell_(false),
    can_correct_(false)
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
    struct archive* aw = archive_write_disk_new();
    struct archive_entry* entry = 0;
    archive_read_support_compression_all(ar);
    archive_read_support_format_all(ar);
    archive_write_disk_set_options(aw, 
                                   ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(aw);
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
        if ((strncmp(filename, "acceptor.", strlen("acceptor.")) == 0) ||
            (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0))
          {
            char* temporary = strdup("/tmp/zhfstaeXXXXXX");
            int temp_fd = mkstemp(temporary);
            archive_entry_set_pathname(entry, temporary);
            close(temp_fd);
            // FIXME: potential race condition
            archive_write_header(aw, entry);
            const void* buf;
            size_t size = 0;
            off_t offset = 0;
            while (rr == ARCHIVE_OK)
              {
                rr = archive_read_data_block(ar, &buf, &size, &offset);
                int rw = archive_write_data_block(aw, buf, size, offset);
                if (rw != ARCHIVE_OK)
                  {
                    throw ZHfstTemporaryWritingError("Writing to temporary"
                                                      "dir failed");
                  }
              }
            if (rr == ARCHIVE_EOF)
              {
                rr = ARCHIVE_OK;
              }
            else 
              {
                throw ZHfstZipReadingError("Archive not EOF'd");
              }

            archive_write_finish_entry(aw);
            char* p = filename;
            if (strncmp(filename, "acceptor.", strlen("acceptor.")) == 0)
                {
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
                  FILE* f = fopen(temporary, "r");
                  if (f == NULL)
                    {
                      throw ZHfstTemporaryWritingError("reading acceptor back "
                                                       "from temp file");
                    }
                  Transducer* trans = new Transducer(f);
                  acceptors_[descr] = trans;
                }
            else if (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0)
                {
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
                  FILE* f = fopen(temporary, "r");
                  if (NULL == f)
                    {
                      throw ZHfstTemporaryWritingError("reading errmodel back "
                                                       "from temp file");
                    }
                  Transducer* trans = new Transducer(f);
                  errmodels_[descr] = trans;
                }
            else
              {
                throw std::logic_error("acceptor || errmodel && "
                                       "!acceptor && !errmodel in substring");
              }
          } // if acceptor or errmodel
        else if (strcmp(filename, "index.xml") == 0)
          {
            char* temporary = strdup("/tmp/zhfstidxXXXXXX");
            int temp_fd = mkstemp(temporary);
            archive_entry_set_pathname(entry, temporary);
            close(temp_fd);
            // FIXME: potential race condition
            archive_write_header(aw, entry);
            const void* buf;
            size_t size = 0;
            off_t offset = 0;
            while (rr == ARCHIVE_OK)
              {
                rr = archive_read_data_block(ar, &buf, &size, &offset);
                int rw = archive_write_data_block(aw, buf, size, offset);
                if (rw != ARCHIVE_OK)
                  {
                    throw ZHfstTemporaryWritingError("Writing to temporary"
                                                      "dir failed");
                  }
              }
            if (rr == ARCHIVE_EOF)
              {
                rr = ARCHIVE_OK;
              }
            else
              {
                throw ZHfstZipReadingError("Archive not EOF'd");
              }
            archive_write_finish_entry(aw);
            metadata_.read_xml(temporary);

          }
        else
          {
            fprintf(stderr, "Unknown file in archive %s\n", filename);
          }
        free(filename);
      } // while r != ARCHIVE_EOF
    archive_read_close(ar);
    archive_write_close(aw);
    archive_write_finish(aw);
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
    

