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

using std::map;

#if HAVE_LIBXML
#  include <libxml++/libxml++.h>
#endif

#include "ospell.h"
#include "hfst-ol.h"

namespace hfst_ol 
  {
    //! @brief data type for associating set of translations to languages.
    typedef std::map<std::string,std::string> LanguageVersions;


    //! @brief ZHfstOspellerInfo represents one info block of an zhfst file.
    //! @seealso https://victorio.uit.no/langtech/trunk/plan/proof/doc/lexfile-spec.xml
    struct ZHfstOspellerInfoMetadata
      {
        //! @brief active locale of speller in BCP format
        std::string locale_;
        //! @brief transalation of titles to all languages
        LanguageVersions title_;
        //! @brief translation of descriptions to all languages
        LanguageVersions description_;
        //! @brief version defintition as free form string
        std::string version_;
        //! @brief vcs revision as string
        std::string vcsrev_;
        //! @brief date for age of speller as string
        std::string date_;
        //! @brief producer of the speller
        std::string producer_;
        //! @brief email address of the speller
        std::string email_;
        //! @brief web address of the speller
        std::string website_;
      };
    //! @brief Represents one acceptor block in XML metadata
    struct ZHfstOspellerAcceptorMetadata
      {
        //! @brief unique id of acceptor
        std::string id_;
        //! @brief descr part of acceptor
        std::string descr_;
        //! @brief type of dictionary
        std::string type_;
        //! @brief type of transducer
        std::string transtype_;
        //! @brief titles of dictionary in languages
        LanguageVersions title_;
        //! @brief descriptions of dictionary in languages
        LanguageVersions description_;
      };
    //! @brief Represents one errmodel block in XML metadata
    struct ZHfstOspellerErrModelMetadata
      {
        //! @brief id of each error model in set
        std::string id_;
        //! @brief descr part of each id
        std::string descr_;
        //! @brief title of error models in languages
        LanguageVersions title_;
        //! @brief description of error models in languages
        LanguageVersions description_;
        std::vector<std::string> type_;
        std::vector<std::string> model_;
      };
    //! @brief holds one index.xml metadata for whole ospeller
    class ZHfstOspellerXmlMetadata
      {
        public:
        //! @brief construct metadata for undefined language and other default
        //!        values
        ZHfstOspellerXmlMetadata();
        void read_xml(const std::string& filename);
        void read_xml(const char* data, size_t data_length);
        std::string debug_dump() const;

        public:
        ZHfstOspellerInfoMetadata info_;
        std::map<std::string,ZHfstOspellerAcceptorMetadata> acceptor_;
        std::vector<ZHfstOspellerErrModelMetadata> errmodel_;
#if HAVE_LIBXML
        private:
        void parse_xml(const xmlpp::Document* doc);
#endif
      };
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
