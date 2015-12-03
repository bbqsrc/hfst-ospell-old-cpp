%module HfstOspell

//%include <std_pair.i>
//%include <std_vector.i>
//%include <std_string.i>

%include <stl.i>
%include <exception.i>

#define int64_t long long
#define uint64_t long long
#define Weight float

%template(StringPair) std::pair<std::string, std::string>;
%template(StringWeightPair) std::pair<std::string, Weight>;
%template(StringPairWeightPair) std::pair<std::pair<std::string, std::string>, Weight>;
%template(StringWeightPairVector) std::vector<std::pair<std::string, Weight> >;
%template(StringPairWeightPairVector) std::vector<std::pair<std::pair<std::string, std::string>, Weight> >;

#define StringWeightPair std::pair<std::string, Weight>
#define StringPairWeightPair std::pair<std::pair<std::string, std::string>, Weight>

%{
#include "hfst-ol.h"
#include "ospell.h"
#include "ZHfstOspeller.h"
%}

%typemap(javabase) hfst_ol::ZHfstException "java.lang.Exception";

%typemap(javacode) hfst_ol::ZHfstException %{
    public String getMessage() {
        return what();
    }
%}

%exception {
    try {
        $action
    } catch (hfst_ol::ZHfstException& e) {
        jclass ex = jenv->FindClass("fi/helsinki/hfst/ZHfstException");
        jenv->ThrowNew(ex, e.what());
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_UnknownError, e.what());
    }
}

%rename("%(lowercamelcase)s") "";
%rename("%(titlecase)s", %$isclass) "";

%ignore hfst_ol::ZHfstOspeller::inject_speller(Speller *s);
%ignore hfst_ol::ZHfstOspeller::get_metadata() const;

// BUG: SWIG bugs up with this method for some reason and makes linker errors
%ignore hfst_ol::ZHfstOspeller::hyphenate(const std::string& wordform);

%include "ZHfstOspeller.h"

