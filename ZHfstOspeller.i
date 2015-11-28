%module HfstOspell

%include <std_pair.i>
%include <std_vector.i>
%include <std_string.i>

#define int64_t long long
#define uint64_t long long
#define Weight short

%template(StringPair) std::pair<std::string, std::string>;
%template(StringWeightPair) std::pair<std::string, Weight>;
%template(StringPairWeightPair) std::pair<std::pair<std::string, std::string>, Weight>;
%template(StringWeightPairVector) std::vector<std::pair<std::string, Weight> >;
%template(StringPairWeightPairVector) std::vector<std::pair<std::pair<std::string, std::string>, Weight> >;

#define StringWeightPair std::pair<std::string, Weight>
#define StringPairWeightPair std::pair<std::pair<std::string, std::string>, Weight>
%{
#include "ZHfstOspeller.h"
%}

%rename("%(lowercamelcase)s") "";
%rename("%(titlecase)s", %$isclass) "";

%ignore hfst_ol::ZHfstOspeller::inject_speller(Speller *s);
%ignore hfst_ol::ZHfstOspeller::get_metadata() const;
%include "ZHfstOspeller.h"
