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

#ifndef HFST_OSPELL_ZHFSTOSPELLERXMLMETADATA_H_
#define HFST_OSPELL_ZHFSTOSPELLERXMLMETADATA_H_ 1

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
        void verify_hfstspeller(xmlpp::Node* hfstspellerNode);
        void parse_info(xmlpp::Node* infoNode);
        void parse_locale(xmlpp::Node* localeNode);
        void parse_title(xmlpp::Node* titleNode);
        void parse_description(xmlpp::Node* descriptionNode);
        void parse_version(xmlpp::Node* versionNode);
        void parse_date(xmlpp::Node* dateNode);
        void parse_producer(xmlpp::Node* producerNode);
        void parse_contact(xmlpp::Node* contactNode);
        void parse_acceptor(xmlpp::Node* acceptorNode);
        void parse_title(xmlpp::Node* titleNode, const std::string& accName);
        void parse_description(xmlpp::Node* descriptionNode,
                               const std::string& accName);
        void parse_errmodel(xmlpp::Node* errmodelNode);
        void parse_title(xmlpp::Node* titleNode, size_t errm_count);
        void parse_description(xmlpp::Node* descriptionNode, size_t errm_count);
        void parse_type(xmlpp::Node* typeNode, size_t errm_count);
        void parse_model(xmlpp::Node* modelNode, size_t errm_count);
#endif
      };
}

#endif // inclusion GUARD
// vim: set ft=cpp.doxygen:
