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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <map>

#include "ospell.h"
#include "hfst-ol.h"
#include "ZHfstOspellerXmlMetadata.h"

namespace hfst_ol 
  {
    //! @brief ZHfstOspeller class holds one speller contained in one
    //!        zhfst file.
    //!        Ospeller can perform all basic writer tool functionality that
    //!        is supporte by the automata in the zhfst archive.
    class ZHfstOspeller
      {
        public:
            //! @brief create speller with default values for undefined 
            //!        language.
            ZHfstOspeller();
            //! @brief destroy all automata used by the speller.
            ~ZHfstOspeller();
            
            //! @brief construct speller from named file containing valid
            //!        zhfst archive.
            void read_zhfst(const std::string& filename);
            //! @breif construct speller from quad of old speller files in a
            //!        directory.
            //! @deprecated just to support old installations
            void read_legacy(const std::string& path);

            //! @brief  check if the given word is spelled correctly
            bool spell(const std::string& wordform);
            //! @brief construct an ordered set of corrections for misspelled
            //!        word form.
            CorrectionQueue suggest(const std::string& wordform);
            //! @brief analyse word form morphologically
            AnalysisQueue analyse(const std::string& wordform);
            //! @brief hyphenate word form
            HyphenationQueue hyphenate(const std::string& wordform);

            //! @brief create string representation of the speller for
            //!        programmer to debug
            std::string metadata_dump() const;
        private:
            //! @brief file or path where the speller came from
            std::string filename_;
            //! @brief upper bound for suggestions generated and given
            unsigned long suggestions_maximum_;
            //! @brief whether automatons loaded yet can be used to check
            //!        spelling
            bool can_spell_;
            //! @brief whether automatons loaded yet can be used to correct
            //!        word forms
            bool can_correct_;
            //! @brief whether automatons loaded yet can be used to analyse
            //!        word forms
            bool can_analyse_;
            //! @brief whether automatons loaded yet can be used to hyphenate
            //!        word forms
            bool can_hyphenate_;
            //! @brief dictionaries loaded
            std::map<std::string, Transducer*> acceptors_;
            //! @brief error models loaded
            std::map<std::string, Transducer*> errmodels_;
            //! @brief pointer to current speller
            Speller* current_speller_;
            //! @brief pointer to current correction model
            Speller* current_sugger_;
            //! @brief pointer to current morphological analyser
            Speller* current_analyser_;
            //! @brief pointer to current hyphenator
            Transducer* current_hyphenator_;
            //! @brief the metadata of loeaded speller
            ZHfstOspellerXmlMetadata metadata_;
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
// vim: set ft=cpp.doxygen:
