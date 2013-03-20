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
#if HAVE_LIBARCHIVE
#  include <archive.h>
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

#if HAVE_LIBARCHIVE
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
            full_length += curr;
          } 
        else if (curr < buffsize)
          {
            fprintf(stderr, "zhfst fail: curr: %zu, buffsize: %zu\n",
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

#if ZHFST_EXTRACT_TO_TMPDIR
static
char*
extract_to_tmp_dir(archive* ar)
  {
    char* rv = strdup("/tmp/zhfstospellXXXXXXXX");
    int temp_fd = mkstemp(rv);
    int rr = archive_read_data_into_fd(ar, temp_fd);
    if ((rr != ARCHIVE_EOF) && (rr != ARCHIVE_OK))
      {
        throw ZHfstZipReadingError("Archive not EOF'd or OK'd");
      }
    close(temp_fd);
    return rv;
  }
#endif

#endif // HAVE_LIBARCHIVE

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
    for (map<string, Transducer*>::iterator acceptor = acceptors_.begin();
         acceptor != acceptors_.end();
         ++acceptor)
      {
        delete acceptor->second;
      }
    for (map<string, Transducer*>::iterator errmodel = errmodels_.begin();
         errmodel != errmodels_.end();
         ++errmodel)
      {
        delete errmodel->second;
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
#if HAVE_LIBARCHIVE
    struct archive* ar = archive_read_new();
    struct archive_entry* entry = 0;
    archive_read_support_compression_all(ar);
    archive_read_support_format_all(ar);
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
#if ZHFST_EXTRACT_TO_TMPDIR
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
#if ZHFST_EXTRACT_TO_TMPDIR
            FILE* f = fopen(temporary, "r");
            if (f == NULL)
              {
                  throw ZHfstTemporaryWritingError("reading acceptor back "
                                                   "from temp file");
              }
            Transducer* trans = new Transducer(f);
#elif ZHFST_EXTRACT_TO_MEM
            Transducer* trans = new Transducer(reinterpret_cast<char*>(full_data));
            free(full_data);
#endif
            acceptors_[descr] = trans;
            free(descr);
          }
        else if (strncmp(filename, "errmodel.", strlen("errmodel.")) == 0)
          {
#if ZHFST_EXTRACT_TO_TMPDIR
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
#if ZHFST_EXTRACT_TO_TMPDIR
            FILE* f = fopen(temporary, "r");
            if (NULL == f)
              {
                throw ZHfstTemporaryWritingError("reading errmodel back "
                                                 "from temp file");
              }
            Transducer* trans = new Transducer(f);
#elif ZHFST_EXTRACT_TO_MEM
            Transducer* trans = new Transducer(reinterpret_cast<char*>(full_data));
            free(full_data);
#endif
            errmodels_[descr] = trans;
            free(descr);
          } // if acceptor or errmodel
        else if (strcmp(filename, "index.xml") == 0)
          {
#if ZHFST_EXTRACT_TO_TMPDIR
            char* temporary = extract_to_tmp_dir(ar);
            metadata_.read_xml(temporary);
#elif ZHFST_EXTRACT_TO_MEM
            size_t xml_len = 0;
            void* full_data = extract_to_mem(ar, entry, &xml_len);
            metadata_.read_xml(reinterpret_cast<char*>(full_data), xml_len);
            free(full_data);
#endif

          }
        else
          {
            fprintf(stderr, "Unknown file in archive %s\n", filename);
          }
        free(filename);
      } // while r != ARCHIVE_EOF
    archive_read_close(ar);
    archive_read_finish(ar);
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
#else
    throw ZHfstZipReadingError("Zip support was disabled");
#endif // HAVE_LIBARCHIVE
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
        fclose(f);
      }
    f = fopen(suggerfile.c_str(), "r");
    if (f != NULL)
      {
        Transducer* trans = new Transducer(f);
        acceptors_["suggestion"] = trans;
        sugger = true;
        fclose(f);
      }
    f = fopen(errorfile.c_str(), "r");
    if (f != NULL)
      {
        Transducer* trans = new Transducer(f);
        errmodels_["default"] = trans;
        error = true;
      }
    if (speller && sugger && error)
      {
        can_spell_ = true;
        can_correct_ = true;
        current_speller_ = new Speller(errmodels_["default"],
                                       acceptors_["default"]);
        current_sugger_ = new Speller(errmodels_["default"],
                                      acceptors_["suggestion"]);
      }
    else if (speller && error)
      {
        can_spell_ = true;
        can_correct_ = true;
        current_speller_ = new Speller(errmodels_["default"],
                                       acceptors_["default"]);
        current_sugger_ = current_speller_;
      }
    else
      {
        throw ZHfstLegacyReadingError("no readable legacy automata found");
      }
  }

const ZHfstOspellerXmlMetadata&
ZHfstOspeller::get_metadata() const
  {
    return metadata_;
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


