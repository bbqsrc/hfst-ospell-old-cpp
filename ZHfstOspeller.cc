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

    ZHfstOspellerInfo::ZHfstOspellerInfo() :
      locale_("und"),
      version_("0"),
      vcsrev_("-1"),
      date_("1970-01-01")
    {}

void
ZHfstOspellerInfo::read_xml(const string& filename)
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
    while ((cur != NULL) && !infoRead)
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
                    if ((locale_ != "und") && 
                        (xmlStrcmp(xmlCharStrdup(locale_.c_str()), data) != 0))
                      {
                        // locale in XML mismatches previous definition
                        // warnable, but overridden as per spec.
                      }
                    locale_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("title")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(info, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        title_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        title_[locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("description")) == 0)
                  {
                    xmlChar* lang = xmlGetProp(info, xmlCharStrdup("lang"));
                    if (lang != NULL)
                      {
                        description_[reinterpret_cast<char*>(lang)] = reinterpret_cast<char*>(data);
                      }
                    else
                      {
                        description_[locale_] = reinterpret_cast<char*>(data);
                      }
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("version")) == 0)
                  {
                    xmlChar* revision = xmlGetProp(info, xmlCharStrdup("vcsrev"));
                    if (revision != NULL)
                      {
                        vcsrev_ = reinterpret_cast<char*>(revision);
                      }
                    version_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("date")) == 0)
                  {
                    date_ = reinterpret_cast<char*>(data);
                  }
                else if (xmlStrcmp(info->name,
                                   xmlCharStrdup("producer")) == 0)
                  {
                    producer_ = reinterpret_cast<char*>(data);
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
                        email_ = reinterpret_cast<char*>(email);
                      }
                    if (website != NULL)
                      {
                        website_ = reinterpret_cast<char*>(website);
                      }
                  }
                info = info->next;
              }
            infoRead = true;
          }
        cur = cur->next;
      }
    xmlFreeDoc(doc);
  }

string
ZHfstOspellerInfo::debug_dump() const
  {
    string retval = "locale: " + locale_ + "\n"
        "version: " + version_ + " [vcsrev: " + vcsrev_ + "]\n"
        "date: " + date_ + "\n"
        "producer: " + producer_ + "[email: <" + email_ + ">, "
        "website: <" + website_ + ">]\n";
    for (map<string,string>::const_iterator title = title_.begin();
         title != title_.end();
         ++title)
      {
        retval.append("title [" + title->first + "]: " + title->second + "\n");
      }
    for (map<string,string>::const_iterator description = description_.begin();
         description != description_.end();
         ++description)
      {
        retval.append("description [" + description->first + "]: " +
            description->second + "\n");
      }
    return retval;
  }

ZHfstOspeller::ZHfstOspeller() :
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
    for (map<string, Transducer*>::iterator t = acceptors_.begin();
         t != acceptors_.end();
         ++t)
      {
        delete t->second;
      }
    for (map<string, Transducer*>::iterator t = errmodels_.begin();
         t != errmodels_.end();
         ++t)
      {
        delete t->second;
      }
  }

bool
    ZHfstOspeller::spell(const string& wordform)
      {
        char* wf = strdup(wordform.c_str());
        return current_speller_->check(wf);
        free(wf);
      }

CorrectionQueue
    ZHfstOspeller::suggest(const string& wordform)
      {
        char* wf = strdup(wordform.c_str());
        return current_sugger_->correct(wf);
        free(wf);
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
        const char* filename = archive_entry_pathname(entry);
        if ((strncmp(filename, "acceptor.", strlen("acceptor.")) == 0) ||
            (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0))
          {
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
            const char* p = filename;
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
                  FILE* f = fopen(filename, "r");
                  if (f == NULL)
                    {
                      throw ZHfstTemporaryWritingError("reading acceptor back "
                                                       "from temp file");
                    }
                  Transducer* trans = new Transducer(f);
                  acceptors_[descr] = trans;
                }
            if (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0)
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
                  FILE* f = fopen(filename, "r");
                  if (NULL == f)
                    {
                      throw ZHfstTemporaryWritingError("reading errmodel back "
                                                       "from temp file");
                    }
                  Transducer* trans = new Transducer(f);
                  errmodels_[descr] = trans;
                }
          } // if acceptor or errmodel
        else if (strcmp(filename, "index.xml") == 0)
          {
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
            metadata_.read_xml(filename);

          }
        else
          {
            fprintf(stderr, "Unknown file in arcgive %s\n", filename);
          }
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
      }
    else
      {
        fprintf(stderr, "Could not find default speller, using %s %s\n",
                acceptors_.begin()->first.c_str(),
                errmodels_.begin()->first.c_str());
        current_speller_ = new Speller(
                                       errmodels_.begin()->second,
                                       acceptors_.begin()->second
                                       );
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
    

