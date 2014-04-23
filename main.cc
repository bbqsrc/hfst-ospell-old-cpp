/*
  
  Copyright 2009 University of Helsinki
  
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
  
  http://www.apache.org/licenses/LICENSE-2.0
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
*/

/*
  This is a toy commandline utility for testing spellers on standard io.
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif
#if HAVE_GETOPT_H
#  include <getopt.h>
#endif

#ifdef WINDOWS
#  include <windows.h>
#endif

#include "ol-exceptions.h"
#include "ospell.h"
#include "ZHfstOspeller.h"

using hfst_ol::ZHfstOspeller;
using hfst_ol::Transducer;

static bool quiet = false;
static bool verbose = false;
static bool analyse = false;
static unsigned long suggs = 0;
#ifdef WINDOWS
  static bool output_to_console = false;
#endif

#ifdef WINDOWS
static std::string wide_string_to_string(const std::wstring & wstr)
{
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)§wstr.size(), NULL, 0, NULL, NULL);
  std::string str( size_needed, 0 );
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
  return str;
}
#endif

static int hfst_fprintf(FILE * stream, const char * format, ...)
{
  va_list args;
  va_start(args, format);
#ifdef WINDOWS
  if (output_to_console && (stream == stdout || stream == stderr))
    {
      char buffer [1024];
      int r = vsprintf(buffer, format, args);
      va_end(args);
      if (r < 0)
        return r;
      HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
      if (stream == stderr)
        stdHandle = GetStdHandle(STD_ERROR_HANDLE);

      std::string pstr(buffer);
      DWORD numWritten = 0;
        int wchars_num =
          MultiByteToWideChar(CP_UTF8 , 0 , pstr.c_str() , -1, NULL , 0 );
        wchar_t* wstr = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8 , 0 ,
                            pstr.c_str() , -1, wstr , wchars_num );
        int retval = WriteConsoleW(stdHandle, wstr, wchars_num-1, &numWritten, NULL);
        delete[] wstr;

        return retval;
    }
  else
    {
      int retval = vfprintf(stream, format, args);
      va_end(args);
      return retval;
    }
#else
  int retval = vfprintf(stream, format, args);
  va_end(args);
  return retval;
#endif
}


bool print_usage(void)
{
    std::cout <<
    "\n" <<
    "Usage: " << PACKAGE_NAME << " [OPTIONS] ERRORSOURCE LEXICON\n" <<
    "       " << PACKAGE_NAME << " [OPTIONS] ZHFST-ARCHIVE\n" <<
    "Run a composition of ERRORSOURCE and LEXICON on standard input and\n" <<
    "print corrected output\n" <<
    "Second form seeks error sources and lexicons from the ZHFST-ARCHIVE\n"
    "\n" <<
    "  -h, --help                  Print this help message\n" <<
    "  -V, --version               Print version information\n" <<
    "  -v, --verbose               Be verbose\n" <<
    "  -q, --quiet                 Don't be verbose (default)\n" <<
    "  -s, --silent                Same as quiet\n" <<
    "  -a, --analyse               Analyse strings and corrections\n" <<
#ifdef WINDOWS
    "  -k, --output-to-console     Print output to console (Windows-specific)" <<
#endif
    "\n" <<
    "\n" <<
    "Report bugs to " << PACKAGE_BUGREPORT << "\n" <<
    "\n";
    return true;
}

bool print_version(void)
{
    std::cout <<
    "\n" <<
    PACKAGE_STRING << std::endl <<
    __DATE__ << " " __TIME__ << std::endl <<
    "copyright (C) 2009 - 2014 University of Helsinki\n";
    return true;
}

bool print_short_help(void)
{
    print_usage();
    return true;
}

void
do_spell(ZHfstOspeller& speller, const std::string& str)
  {
    if (speller.spell(str)) 
      {
        (void)hfst_fprintf(stdout, "\"%s\" is in the lexicon "
                           "(but correcting anyways)\n\n", str.c_str());
      }
    if (analyse)
      {
        hfst_ol::AnalysisQueue anals = speller.analyse(str, false);
        (void)hfst_fprintf(stdout, "Analyses for \"%s\":\n", str.c_str());
        while (anals.size() > 0)
          {
            (void)hfst_fprintf(stdout, "%s   %f\n",
                               anals.top().first.c_str(),
                               anals.top().second);
            anals.pop();
          }
      }
    hfst_ol::CorrectionQueue corrections = speller.suggest(str);
    if (corrections.size() > 0) 
      {
        (void)hfst_fprintf(stdout, "Corrections for \"%s\":\n", str.c_str());
        while (corrections.size() > 0)
          {
            const std::string& corr = corrections.top().first;
            (void)hfst_fprintf(stdout, "%s    %f\n", 
                           corr.c_str(), 
                           corrections.top().second);
            if (analyse)
              {
                hfst_ol::AnalysisQueue anals = speller.analyse(corr, true);
                (void)hfst_fprintf(stdout, "Analyses for \"%s\":\n", 
                                   corr.c_str());
                while (anals.size() > 0)
                  {
                    (void)hfst_fprintf(stdout, "%s\n",
                                       anals.top().first.c_str());
                    anals.pop();
                  }

              }
            corrections.pop();
            (void)hfst_fprintf(stdout, "\n");
          }
        (void)hfst_fprintf(stdout, "\n");
      }
    else
      {
        (void)hfst_fprintf(stdout,
                           "Unable to correct \"%s\"!\n\n", str.c_str());
      }
  }

int
zhfst_spell(char* zhfst_filename)
{
  ZHfstOspeller speller;
  try
    {
      speller.read_zhfst(zhfst_filename);
    }
  catch (hfst_ol::ZHfstMetaDataParsingError zhmdpe)
    {
      (void)hfst_fprintf(stderr, "cannot finish reading zhfst archive %s:\n%s.\n", 
                         zhfst_filename, zhmdpe.what());
      //std::cerr << "cannot finish reading zhfst archive " << zhfst_filename <<
      //             ":\n" << zhmdpe.what() << "." << std::endl;
      return EXIT_FAILURE;
    }
  catch (hfst_ol::ZHfstZipReadingError zhzre)
    {
      //std::cerr << "cannot read zhfst archive " << zhfst_filename << ":\n" 
      //    << zhzre.what() << "." << std::endl
      //    << "trying to read as legacy automata directory" << std::endl;
      (void)hfst_fprintf(stderr, 
                         "cannot read zhfst archive %s:\n"
                         "%s.\n",
                         zhfst_filename, zhzre.what());
      return EXIT_FAILURE;
    }
  catch (hfst_ol::ZHfstXmlParsingError zhxpe)
    {
      //std::cerr << "Cannot finish reading index.xml from " 
      //  << zhfst_filename << ":" << std::endl
      //  << zhxpe.what() << "." << std::endl;
      (void)hfst_fprintf(stderr, 
                         "Cannot finish reading index.xml from %s:\n"
                         "%s.\n", 
                         zhfst_filename, zhxpe.what());
      return EXIT_FAILURE;
    }
  if (verbose)
    {
      //std::cout << "Following metadata was read from ZHFST archive:" << std::endl
      //          << speller.metadata_dump() << std::endl;
      (void)hfst_fprintf(stdout, 
                         "Following metadata was read from ZHFST archive:\n"
                         "%s\n", 
                         speller.metadata_dump().c_str());
    }
  speller.set_queue_limit(suggs);
  if (verbose)
    {
      hfst_fprintf(stdout, "Printing only %lu top suggestions per line", suggs);
    }
  char * str = (char*) malloc(2000);

#ifdef WINDOWS
    SetConsoleCP(65001);
    const HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
    WCHAR buffer[0x1000];
    DWORD numRead = 0;
    while (ReadConsoleW(stdIn, buffer, sizeof buffer, &numRead, NULL))
      {
        std::wstring wstr(buffer, numRead-1); // skip the newline
        std::string linestr = wide_string_to_string(wstr);
        free(str);
        str = strdup(linestr.c_str());
#else    
    while (!std::cin.eof()) {
        std::cin.getline(str, 2000);
#endif
        if (str[0] == '\0') {
            continue;
        }
        if (str[strlen(str) - 1] == '\r')
          {
#ifdef WINDOWS
            str[strlen(str) - 1] = '\0';
#else
            (void)hfst_fprintf(stderr, "There is a WINDOWS linebreak in this file\n"
                               "Please convert with dos2unix or fromdos\n");
            exit(1);
#endif
          }
        do_spell(speller, str);
      }
    free(str);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    
    int c;
    //std::locale::global(std::locale(""));
  
#if HAVE_GETOPT_H
    while (true) {
        static struct option long_options[] =
            {
            // first the hfst-mandated options
            {"help",         no_argument,       0, 'h'},
            {"version",      no_argument,       0, 'V'},
            {"verbose",      no_argument,       0, 'v'},
            {"quiet",        no_argument,       0, 'q'},
            {"silent",       no_argument,       0, 's'},
            {"analyse",      no_argument,       0, 'a'},
            {"limit",  required_argument,       0, 'n'},
#ifdef WINDOWS
            {"output-to-console",       no_argument,       0, 'k'},
#endif
            {0,              0,                 0,  0 }
            };
          
        int option_index = 0;
        c = getopt_long(argc, argv, "hVvqskan:", long_options, &option_index);
        char* endptr = 0;

        if (c == -1) // no more options to look at
            break;

        switch (c) {
        case 'h':
            print_usage();
            return EXIT_SUCCESS;
            break;
          
        case 'V':
            print_version();
            return EXIT_SUCCESS;
            break;
          
        case 'v':
            verbose = true;
            quiet = false;
            break;
          
        case 'q': // fallthrough
        case 's':
            quiet = true;
            verbose = false;
            break;
        case 'a':
            analyse = true;
            break;
        case 'n':
            suggs = strtoul(optarg, &endptr, 10);
            if (endptr == optarg)
              {
                fprintf(stderr, "%s not a strtoul number\n", optarg);
                exit(1);
              }
            else if (*endptr != '\0')
              {
                fprintf(stderr, "%s truncated from limit parameter\n", endptr);
              }
            break;
#ifdef WINDOWS
        case 'k':
            output_to_console = true;
            break;
#endif 
        default:
            std::cerr << "Invalid option\n\n";
            print_short_help();
            return EXIT_FAILURE;
            break;
        }
    }
#else
    int optind = 1;
#endif
    // no more options, we should now be at the input filenames
    if (optind == (argc - 1))
      {
        return zhfst_spell(argv[optind]);
      }
    else if (optind < (argc - 1))
      {
        std::cerr << "Too many file parameters" << std::endl;
        print_short_help();
        return EXIT_FAILURE;
      }
    else if (optind >= argc)
      {
        std::cerr << "Give full path to a zhfst speller"
            << std::endl;
        print_short_help();
        return EXIT_FAILURE;
      }
    return EXIT_SUCCESS;
  }
