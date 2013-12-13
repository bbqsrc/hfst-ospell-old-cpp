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
#include "ol-exceptions.h"
#include "ospell.h"
#include "ZHfstOspeller.h"

using hfst_ol::ZHfstOspeller;
using hfst_ol::Transducer;

static bool quiet = false;
static bool verbose = false;

bool print_usage(void)
{
    std::cerr <<
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
    "\n" <<
    "\n" <<
    "Report bugs to " << PACKAGE_BUGREPORT << "\n" <<
    "\n";
    return true;
}

bool print_version(void)
{
    std::cerr <<
    "\n" <<
    PACKAGE_STRING << std::endl <<
    __DATE__ << " " __TIME__ << std::endl <<
    "copyright (C) 2009 - 2011 University of Helsinki\n";
    return true;
}

bool print_short_help(void)
{
    print_usage();
    return true;
}

int
legacy_spell(const char* errmodel_filename, const char* acceptor_filename)
{
    FILE* mutator_file = fopen(errmodel_filename, "r");
    if (mutator_file == NULL) {
        std::cerr << "Could not open file " << errmodel_filename
              << std::endl;
        return EXIT_FAILURE;
    }
    FILE* lexicon_file = fopen(acceptor_filename, "r");
    if (lexicon_file == NULL) {
        std::cerr << "Could not open file " << acceptor_filename
              << std::endl;
        return EXIT_FAILURE;
    }
    hfst_ol::Transducer * mutator;
    hfst_ol::Transducer * lexicon;
    mutator = new hfst_ol::Transducer(mutator_file);
    if (!mutator->is_weighted()) {
        std::cerr << "Error source was unweighted, exiting\n\n";
        return EXIT_FAILURE;
    }
    lexicon = new hfst_ol::Transducer(lexicon_file);
    if (!lexicon->is_weighted()) {
        std::cerr << "Lexicon was unweighted, exiting\n\n";
        return EXIT_FAILURE;
    }
    
    hfst_ol::Speller * speller;

    try {
        speller = new hfst_ol::Speller(mutator, lexicon);
    } catch (hfst_ol::AlphabetTranslationException& e) {
        std::cerr <<
        "Unable to build speller - symbol " << e.what() << " not "
        "present in lexicon's alphabet\n";
        return EXIT_FAILURE;
    }
    std::cout << "This is basic HFST spellchecker read in legacy " 
        "mode; use zhfst version to test new format" << std::endl;
    char * str = (char*) malloc(2000);
    
    while (!std::cin.eof()) {
        std::cin.getline(str, 2000);
        if (str[0] == '\0') 
          {
            std::cerr << "Skipping empty lines" << std::endl;
            continue;
          }
        if (str[strlen(str) - 1] == '\r')
          {
            std::cerr << "There is a WINDOWS linebreak in this file" <<
                std::endl <<
                "Please convert with dos2unix or fromdos" << std::endl;
            exit(1);
          }
        if (speller->check(str)) {
        std::cout << "\"" << str << "\" is in the lexicon\n\n";
        } else {
        hfst_ol::CorrectionQueue corrections = speller->correct(str, 5);
        if (corrections.size() > 0) {
            std::cout << "Corrections for \"" << str << "\":\n";
            while (corrections.size() > 0)
            {
            std::cout << corrections.top().first << "    " << corrections.top().second << std::endl;
            corrections.pop();
            }
            std::cout << std::endl;
        } else {
            std::cout << "Unable to correct \"" << str << "\"!\n\n";
        }
        }
    }
    return EXIT_SUCCESS;

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
      std::cerr << "cannot finish reading zhfst archive " << zhfst_filename <<
                   ":\n" << zhmdpe.what() << "." << std::endl;
      return EXIT_FAILURE;
    }
  catch (hfst_ol::ZHfstZipReadingError zhzre)
    {
      std::cerr << "cannot read zhfst archive " << zhfst_filename << ":\n" 
          << zhzre.what() << "." << std::endl
          << "trying to read as legacy automata directory" << std::endl;
      try 
        {
          speller.read_legacy(zhfst_filename);
        }
      catch (hfst_ol::ZHfstLegacyReadingError zhlre)
        {
          std::cerr << "cannot fallback to read legacy hfst speller dir " 
              << zhfst_filename 
              << ":\n" << std::endl
              << zhlre.what() << "." << std::endl;
          return EXIT_FAILURE;
        }
    }
  catch (hfst_ol::ZHfstXmlParsingError zhxpe)
    {
      std::cerr << "Cannot finish reading index.xml from " 
        << zhfst_filename << ":" << std::endl
        << zhxpe.what() << "." << std::endl;
      return EXIT_FAILURE;
    }
  if (verbose)
    {
      std::cout << "Following metadata was read from ZHFST archive:" << std::endl
                << speller.metadata_dump() << std::endl;
    }
  char * str = (char*) malloc(2000);
    
    while (!std::cin.eof()) {
        std::cin.getline(str, 2000);
        if (str[0] == '\0') {
        break;
        }
        if (str[strlen(str) - 1] == '\r')
          {
            std::cerr << "There is a WINDOWS linebreak in this file" <<
                std::endl <<
                "Please convert with dos2unix or fromdos" << std::endl;
            exit(1);
          }
        if (speller.spell(str)) {
        std::cout << "\"" << str << "\" is in the lexicon\n\n";
        } else {
        hfst_ol::CorrectionQueue corrections = speller.suggest(str);
        if (corrections.size() > 0) {
            std::cout << "Corrections for \"" << str << "\":\n";
            while (corrections.size() > 0)
            {
            std::cout << corrections.top().first << "    " << corrections.top().second << std::endl;
            corrections.pop();
            }
            std::cout << std::endl;
        } else {
            std::cout << "Unable to correct \"" << str << "\"!\n\n";
        }
        }
    }
    free(str);
    return EXIT_SUCCESS;
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
            {0,              0,                 0,  0 }
            };
          
        int option_index = 0;
        c = getopt_long(argc, argv, "hVvqs", long_options, &option_index);

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
    if (optind == (argc - 2))
      {
        return legacy_spell(argv[optind], argv[optind+1]);
      }
    else if (optind == (argc - 1))
      {
        return zhfst_spell(argv[optind]);
      }
    else if (optind < (argc - 2))
      {
        std::cerr << "No more than two free parameters allowed" << std::endl;
        print_short_help();
        return EXIT_FAILURE;
      }
    else if (optind >= argc)
      {
        std::cerr << "Give full path to zhfst spellers or two automata"
            << std::endl;
        print_short_help();
        return EXIT_FAILURE;
      }
    return EXIT_SUCCESS;
  }
