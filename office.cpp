/*

  Copyright 2009 University of Helsinki
  Copyright 2015 Tino Didriksen <mail@tinodidriksen.com>
  Code adapted from https://github.com/TinoDidriksen/trie-tools

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
	Tests up to 8 variations of each input token:
	- Verbatim
	- With leading non-alphanumerics removed
	- With trailing non-alphanumerics removed
	- With leading and trailing non-alphanumerics removed
	- Lower-case of all the above
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cerrno>
#include <cctype>
#include <unicode/uclean.h>
#include <unicode/ucnv.h>
#include <unicode/uloc.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include "ol-exceptions.h"
#include "ospell.h"
#include "ZHfstOspeller.h"

using hfst_ol::ZHfstOspeller;
using hfst_ol::Transducer;

typedef std::map<UnicodeString,bool> valid_words_t;
valid_words_t valid_words;

struct word_t {
	size_t start, count;
	UnicodeString buffer;
};
std::vector<word_t> words(8);
std::string buffer;
UnicodeString ubuffer;
size_t cw;

bool verbatim = false;

bool find_alternatives(ZHfstOspeller& speller, size_t suggs) {
	/* Weights make this entirely pointless
	size_t dist = std::max(static_cast<size_t>(1), static_cast<size_t>(std::log(words[cw-1].buffer.size()) / std::log(2)));
	speller.set_weight_limit(dist);
	speller.set_beam(dist / 2);
	//*/

	for (size_t k=1 ; k <= cw ; ++k) {
		buffer.clear();
		words[cw-k].buffer.toUTF8String(buffer);
		hfst_ol::CorrectionQueue corrections = speller.suggest(buffer);

		if (corrections.size() == 0) {
			continue;
		}

		std::cout << "&";
		// Because speller.set_queue_limit() doesn't actually work, hard limit it here
		for (size_t i=0, e=corrections.size() ; i<e && i<suggs ; ++i) {
			std::cout << "\t";

			buffer.clear();
			if (cw - k != 0) {
				words[0].buffer.tempSubString(0, words[cw-k].start).toUTF8String(buffer);
			}
			buffer.append(corrections.top().first);
			if (cw - k != 0) {
				words[0].buffer.tempSubString(words[cw-k].start + words[cw-k].count).toUTF8String(buffer);
			}

			std::cout << buffer;
			corrections.pop();
		}
		std::cout << std::endl;
		return true;
	}
	return false;
}

bool is_valid_word(ZHfstOspeller& speller, const std::string& word) {
	ubuffer.setTo(UnicodeString::fromUTF8(word));

	bool has_letters = false;
	for (int32_t i=0 ; i<ubuffer.length() ; ++i) {
		if (u_isalpha(ubuffer[i])) {
			has_letters = true;
			break;
		}
	}

	// If there are no letters in this token, just ignore it
	if (has_letters == false) {
		return true;
	}

	size_t ichStart = 0, cchUse = ubuffer.length();
	const UChar *pwsz = ubuffer.getTerminatedBuffer();

	// Always test the full given input
	words[0].buffer.remove();
	words[0].start = ichStart;
	words[0].count = cchUse;
	words[0].buffer = ubuffer;
	cw = 1;

	if (cchUse > 1 && !verbatim) {
		size_t count = cchUse;
		while (count && !u_isalnum(pwsz[ichStart+count-1])) {
			--count;
		}
		if (count != cchUse) {
			// If the input ended with non-alphanumerics, test input with non-alphanumerics trimmed from the end
			words[cw].buffer.remove();
			words[cw].start = ichStart;
			words[cw].count = count;
			words[cw].buffer.append(pwsz, words[cw].start, words[cw].count);
			++cw;
		}

		size_t start = ichStart, count2 = cchUse;
		while (start < ichStart+cchUse && !u_isalnum(pwsz[start])) {
			++start;
			--count2;
		}
		if (start != ichStart) {
			// If the input started with non-alphanumerics, test input with non-alphanumerics trimmed from the start
			words[cw].buffer.remove();
			words[cw].start = start;
			words[cw].count = count2;
			words[cw].buffer.append(pwsz, words[cw].start, words[cw].count);
			++cw;
		}

		if (start != ichStart && count != cchUse) {
			// If the input both started and ended with non-alphanumerics, test input with non-alphanumerics trimmed from both sides
			words[cw].buffer.remove();
			words[cw].start = start;
			words[cw].count = count - (cchUse - count2);
			words[cw].buffer.append(pwsz, words[cw].start, words[cw].count);
			++cw;
		}
	}

	for (size_t i=0, e=cw ; i<e ; ++i) {
		valid_words_t::iterator it = valid_words.find(words[i].buffer);

		if (it == valid_words.end()) {
			buffer.clear();
			words[i].buffer.toUTF8String(buffer);
			bool valid = speller.spell(buffer);
			it = valid_words.insert(std::make_pair(words[i].buffer,valid)).first;

			if (!valid && !verbatim) {
				// If the word was not valid, fold it to lower case and try again
				buffer.clear();
				ubuffer = words[i].buffer;
				ubuffer.toLower();
				ubuffer.toUTF8String(buffer);

				// Add the lower case variant to the list so that we get suggestions using that, if need be
				words[cw].start = words[i].start;
				words[cw].count = words[i].count;
				words[cw].buffer = ubuffer;
				++cw;

				// Don't try again if the lower cased variant has already been tried
				valid_words_t::iterator itl = valid_words.find(ubuffer);
				if (itl != valid_words.end()) {
					it->second = itl->second;
					it = itl;
				}
				else {
					valid = speller.spell(buffer);
					it->second = valid; // Also mark the original mixed case variant as whatever the lower cased one was
					it = valid_words.insert(std::make_pair(words[i].buffer,valid)).first;
				}
			}
		}

		if (it->second == true) {
			return true;
		}
	}

	return false;
}

int zhfst_spell(const char* zhfst_filename) {
	ZHfstOspeller speller;
	try {
		speller.read_zhfst(zhfst_filename);
	}
	catch (hfst_ol::ZHfstMetaDataParsingError zhmdpe) {
		fprintf(stderr, "cannot finish reading zhfst archive %s:\n%s.\n", zhfst_filename, zhmdpe.what());
		return EXIT_FAILURE;
	}
	catch (hfst_ol::ZHfstZipReadingError zhzre) {
		fprintf(stderr, "cannot read zhfst archive %s:\n%s.\n", zhfst_filename, zhzre.what());
		return EXIT_FAILURE;
	}
	catch (hfst_ol::ZHfstXmlParsingError zhxpe) {
		fprintf(stderr, "Cannot finish reading index.xml from %s:\n%s.\n", zhfst_filename, zhxpe.what());
		return EXIT_FAILURE;
	}

	std::cout << "@@ hfst-ospell-office is alive" << std::endl;

	std::string line;
	std::string word;
	std::istringstream ss;
	while (std::getline(std::cin, line)) {
		while (!line.empty() && std::isspace(line[line.size()-1])) {
			line.resize(line.size()-1);
		}
		if (line.empty()) {
			continue;
		}
		// Just in case anyone decides to use the speller for a minor eternity
		if (valid_words.size() > 20480) {
			valid_words.clear();
		}

		ss.clear();
		ss.str(line);
		size_t suggs = 0;
		char c = 0;
		if (!(ss >> suggs) || !ss.get(c) || !std::getline(ss, line)) {
			std::cout << "!" << std::endl;
			continue;
		}

		if (is_valid_word(speller, line)) {
			std::cout << "*" << std::endl;
			continue;
		}

		if (!suggs || !find_alternatives(speller, suggs)) {
			std::cout << "#" << std::endl;
		}
	}
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	UErrorCode status = U_ZERO_ERROR;
	u_init(&status);
	if (U_FAILURE(status) && status != U_FILE_ACCESS_ERROR) {
		std::cerr << "Error: Cannot initialize ICU. Status = " << u_errorName(status) << std::endl;
		return -1;
	}

	ucnv_setDefaultName("UTF-8");
	uloc_setDefault("en_US_POSIX", &status);

	std::vector<std::string> args(argv, argv+argc);
	for (std::vector<std::string>::iterator it=args.begin() ; it != args.end() ; ) {
		if (*it == "--verbatim") {
			verbatim = true;
			it = args.erase(it);
		}
		else {
			++it;
		}
	}

	if (args.size() < 2) {
		throw std::invalid_argument("Must pass a zhfst as argument");
	}

	int rv = zhfst_spell(args[1].c_str());

	u_cleanup();
	return rv;
}
