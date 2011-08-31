#ifndef _OL_EXCEPTIONS_H
#define _OL_EXCEPTIONS_H

#include <string>
#include <sstream>

// This structure is inherited from for each exception. Taken from HFST library
// code.
struct OspellException
{
    std::string name;
    std::string file;
    size_t line;

    OspellException(void) {}
    
OspellException(const std::string &name,const std::string &file,size_t line):
    name(name),
	file(file),
	line(line)
	{}
    
    std::string operator() (void) const
	{
	    std::ostringstream o;
	    o << "Exception: "<< name << " in file: "
	      << file << " on line: " << line;
	    return o.str();
	}
};

// These macros are used instead of the normal exception facilities.

#define HFST_THROW(E) throw E(#E,__FILE__,__LINE__)

#define HFST_THROW_MESSAGE(E,M) throw E(std::string(#E)+": "+std::string(M)\
                        ,__FILE__,__LINE__)

#define HFST_EXCEPTION_CHILD_DECLARATION(CHILD) \
    struct CHILD : public OspellException \
    { CHILD(const std::string &name,const std::string &file,size_t line):\
	OspellException(name,file,line) {}} 

#define HFST_CATCH(E)							\
    catch (const E &e)							\
    {									\
    std::cerr << e.file << ", line " << e.line << ": " <<       \
        e() << std::endl;                       \
    }

// Now the exceptions themselves

HFST_EXCEPTION_CHILD_DECLARATION(HeaderParsingException);

HFST_EXCEPTION_CHILD_DECLARATION(AlphabetParsingException);

HFST_EXCEPTION_CHILD_DECLARATION(IndexTableReadingException);

HFST_EXCEPTION_CHILD_DECLARATION(TransitionTableReadingException);

HFST_EXCEPTION_CHILD_DECLARATION(UnweightedSpellerException);

HFST_EXCEPTION_CHILD_DECLARATION(TransducerTypeException);

#endif // _OL_EXCEPTIONS_H
