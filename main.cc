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

#include "ospell.h"

#define PACKAGE_NAME "hfst-ospell"
#define PACKAGE_STRING "hfst-ospell 0.1"
#define PACKAGE_BUGREPORT "hfst-bugs@ling.helsinki.fi"

bool print_usage(void)
{
    std::cerr <<
	"\n" <<
	"Usage: " << PACKAGE_NAME << " [OPTIONS] ERRORSOURCE LEXICON\n" <<
	"Run a composition of ERRORSOURCE and LEXICON on standard input and" <<
	"print corrected output\n" <<
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
	"copyright (C) 2009 University of Helsinki\n";
    return true;
}

bool print_short_help(void)
{
    print_usage();
    return true;
}

int main(int argc, char **argv)
{

    FILE * mutator_file = NULL;
    FILE * lexicon_file = NULL;
    
    int c;
  
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
	    break;
	  
	case 'q': // fallthrough
	case 's':
	    break;
	  
	default:
	    std::cerr << "Invalid option\n\n";
	    print_short_help();
	    return EXIT_FAILURE;
	    break;
	}
    }
    // no more options, we should now be at the input filenames
    if ( (optind + 2) < argc) {
	std::cerr << "More than two input files given\n";
	return EXIT_FAILURE;
    } else if ( (optind + 2) > argc)
    {
	std::cerr << "Need two input files\n";
	return EXIT_FAILURE;
    } else {
	mutator_file = fopen(argv[(optind)], "r");
	if (mutator_file == NULL) {
	    std::cerr << "Could not open file " << argv[(optind)] << std::endl;
	    return 1;
	}
	lexicon_file = fopen(argv[(optind + 1)], "r");
	if (lexicon_file == NULL) {
	    std::cerr << "Could not open file " << argv[(optind)] << std::endl;
	    return 1;
	}
	hfst_ol::Transducer * mutator;
	hfst_ol::Transducer * lexicon;
	try {
	    mutator = new hfst_ol::Transducer(mutator_file);
	} catch (hfst_ol::UnweightedSpellerException e) {
	    std::cerr << "Error source was unweighted, exiting\n\n";
	    return EXIT_FAILURE;
	}
	try {
	    lexicon = new hfst_ol::Transducer(lexicon_file);
	} catch (hfst_ol::UnweightedSpellerException e) {
	    std::cerr << "Lexicon was unweighted, exiting\n\n";
	    return EXIT_FAILURE;
	}
	hfst_ol::Speller speller(mutator, lexicon);
	
	char * str = (char*) malloc(2000);
	
	while (!std::cin.eof()) {
	    std::cin.getline(str, 2000);
	    if (speller.check(str)) {
		std::cout << "\"" << str << "\" is in the lexicon\n\n";
	    } else {
		hfst_ol::CorrectionQueue corrections = speller.correct(str);
		if (corrections.size() > 0) {
		    std::cout << "Corrections for \"" << str << "\":\n";
		    while (corrections.size() > 0)
		    {
			std::cout << corrections.top().first << "\t" << corrections.top().second << std::endl;
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
}
