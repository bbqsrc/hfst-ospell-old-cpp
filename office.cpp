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
	- With leading punctuaton removed
	- With trailing punctuaton removed
	- With leading and trailing punctuaton removed
	- Lower-case of all the above
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <cwctype>
#include <cmath>
#include <cerrno>

#include "ol-exceptions.h"
#include "ospell.h"
#include "ZHfstOspeller.h"

using hfst_ol::ZHfstOspeller;
using hfst_ol::Transducer;

typedef std::unordered_map<std::string,bool> valid_words_t;
valid_words_t valid_words;

struct word_t {
	size_t start, count;
	std::string buffer;
};
std::vector<word_t> words(8);
std::string buffer;
size_t cw;

void find_alternatives(ZHfstOspeller& speller, const std::string& word) {
	/* Weights make this entirely pointless
	size_t dist = std::max(static_cast<size_t>(1), static_cast<size_t>(std::log(words[cw-1].buffer.size()) / std::log(2)));
	speller.set_weight_limit(dist);
	speller.set_beam(dist / 2);
	//*/

	for (size_t k=1 ; cw-k >= 0 ; ++k) {
		hfst_ol::CorrectionQueue corrections = speller.suggest(words[cw-k].buffer);

		if (corrections.size() == 0) {
			continue;
		}

		std::cout << "& " << word << " " << corrections.size() << " 0" << ": ";
		// Because speller.set_queue_limit() doesn't actually work, hard limit it here
		for (size_t i=0, e=corrections.size() ; i<e && i<30 ; ++i) {
			if (i != 0) {
				std::cout << ", ";
			}

			buffer.clear();
			if (cw - k != 0) {
				buffer.append(words[0].buffer.begin(), words[0].buffer.begin() + words[cw-k].start);
			}
			buffer.append(corrections.top().first);
			if (cw - k != 0) {
				buffer.append(words[0].buffer.begin() + words[cw-k].start + words[cw-k].count, words[0].buffer.end());
			}

			std::cout << buffer;
			corrections.pop();
		}
		std::cout << std::endl;
		return;
	}

	std::cout << "# " << word << " 0" << std::endl;
}

bool is_valid_word(ZHfstOspeller& speller, const std::string& word) {
	size_t ichStart = 0, cchUse = word.size();
	const char *pwsz = word.c_str();

	// Always test the full given input
	words[0].buffer.resize(0);
	words[0].start = ichStart;
	words[0].count = cchUse;
	words[0].buffer.append(pwsz+ichStart, pwsz+ichStart+cchUse);
	cw = 1;

	if (cchUse > 1) {
		size_t count = cchUse;
		while (count && (std::iswpunct(pwsz[ichStart+count-1]) || std::iswspace(pwsz[ichStart+count-1]))) {
			--count;
		}
		if (count != cchUse) {
			// If the input ended with punctuation, test input with punctuation trimmed from the end
			words[cw].buffer.resize(0);
			words[cw].start = ichStart;
			words[cw].count = count;
			words[cw].buffer.append(pwsz+words[cw].start, pwsz+words[cw].start+words[cw].count);
			++cw;
		}

		size_t start = ichStart, count2 = cchUse;
		while (start < ichStart+cchUse && (std::iswpunct(pwsz[start]) || std::iswspace(pwsz[start]))) {
			++start;
			--count2;
		}
		if (start != ichStart) {
			// If the input started with punctuation, test input with punctuation trimmed from the start
			words[cw].buffer.resize(0);
			words[cw].start = start;
			words[cw].count = count2;
			words[cw].buffer.append(pwsz+words[cw].start, pwsz+words[cw].start+words[cw].count);
			++cw;
		}

		if (start != ichStart && count != cchUse) {
			// If the input both started and ended with punctuation, test input with punctuation trimmed from both sides
			words[cw].buffer.resize(0);
			words[cw].start = start;
			words[cw].count = count - (cchUse - count2);
			words[cw].buffer.append(pwsz+words[cw].start, pwsz+words[cw].start+words[cw].count);
			++cw;
		}
	}

	for (size_t i=0, e=cw ; i<e ; ++i) {
		typename valid_words_t::iterator it = valid_words.find(words[i].buffer);

		if (it == valid_words.end()) {
			bool valid = speller.spell(words[i].buffer);
			it = valid_words.insert(std::make_pair(words[i].buffer,valid)).first;

			if (!valid) {
				// If the word was not valid, fold it to lower case and try again
				buffer.resize(0);
				buffer.reserve(words[i].buffer.size());
				std::transform(words[i].buffer.begin(), words[i].buffer.end(), std::back_inserter(buffer), std::towlower);

				// Add the lower case variant to the list so that we get suggestions using that, if need be
				words[cw].start = words[i].start;
				words[cw].count = words[i].count;
				words[cw].buffer = buffer;
				++cw;

				// Don't try again if the lower cased variant has already been tried
				typename valid_words_t::iterator itl = valid_words.find(buffer);
				if (itl != valid_words.end()) {
					it->second = itl->second;
					it = itl;
				}
				else {
					valid = speller.spell(buffer);
					it->second = valid; // Also mark the original mixed case variant as whatever the lower cased one was
					it = valid_words.insert(std::make_pair(buffer,valid)).first;
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

	std::cout << "@(#) International Ispell Version 3.1.20 (but really hfst-ospell-office)" << std::endl;

	std::string line;
	std::string word;
	while (std::getline(std::cin, line)) {
		while (!line.empty() && std::iswspace(line[line.size()-1])) {
			line.resize(line.size()-1);
		}
		if (line.empty()) {
			continue;
		}

		if (is_valid_word(speller, line)) {
			std::cout << "*" << std::endl;
			continue;
		}

		find_alternatives(speller, line);
	}
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	return zhfst_spell(argv[1]);
}
