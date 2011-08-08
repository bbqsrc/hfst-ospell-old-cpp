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

#ifndef HFST_OSPELL_ZHFSTOSPELLER_H_
#define HFST_OSPELL_ZHFSTOSPELLER_H_

#include <map>

using std::map;


#include "ospell.h"
#include "hfst-ol.h"

namespace hfst_ol 
  {
    //! @brief HfstOspellerInfo represents one info block of an zhfst file.
    //! @seealso https://victorio.uit.no/langtech/trunk/plan/proof/doc/lexfile-spec.xml
    class ZHfstOspellerInfo
      {
        public:
            //! @brief construct empty speller info for undefined language and
            //! no other metadata.
            ZHfstOspellerInfo();
            //! @brief read metadate from XML file laid out like the spec.
            void read_xml(const std::string& filename);
            std::string debug_dump() const;
        private:
            std::string locale_;
            std::map<std::string,std::string> title_;
            std::map<std::string,std::string> description_;
            std::string version_;
            std::string vcsrev_;
            std::string date_;
            std::string producer_;
            std::string email_;
            std::string website_;
            friend class ZHfstOspeller;
      };

    class ZHfstOspeller
      {
        public:
            ZHfstOspeller();
            ~ZHfstOspeller();
            
            void read_zhfst(const std::string& filename);
            void read_legacy(const std::string& path);

            bool spell(const std::string& wordform);
            CorrectionQueue suggest(const std::string& wordform);
            
            std::string metadata_dump() const;
        private:
            std::string filename_;
            unsigned long suggestions_;
            bool can_spell_;
            bool can_correct_;
            std::map<std::string, Transducer*> acceptors_;
            std::map<std::string, Transducer*> errmodels_;
            Speller* current_speller_;
            Speller* current_sugger_;
            ZHfstOspellerInfo metadata_;
      };

    class ZHfstException
      {
        public:
            ZHfstException();
            explicit ZHfstException(const std::string& message);
            const char* what();
        private:
            std::string what_;
      };

    class ZHfstMetaDataParsingError : public ZHfstException
    {
        public:
            explicit ZHfstMetaDataParsingError(const std::string& message);
        private:
            std::string what_;
    };

    class ZHfstXmlParsingError : public ZHfstException
    {
      public:
          explicit ZHfstXmlParsingError(const std::string& message);
      private:
          std::string what_;
    };

    class ZHfstZipReadingError : public ZHfstException
    {
      public:
          explicit ZHfstZipReadingError(const std::string& message);
      private:
          std::string what_;
    };

    class ZHfstTemporaryWritingError : public ZHfstException
    {
      public:
          explicit ZHfstTemporaryWritingError(const std::string& message);
      private:
          std::string what_;
    };

    class ZHfstLegacyReadingError : public ZHfstException
    {
      public:
          explicit ZHfstLegacyReadingError(const std::string& message);
      private:
          std::string what_;
    };
  } // namespace hfst_ol


#endif // HFST_OSPELL_OSPELLER_SET_H_
