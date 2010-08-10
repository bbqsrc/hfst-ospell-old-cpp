#include "hfst-ol.h"

namespace hfst_ol {
    
void TransducerAlphabet::get_next_symbol(FILE * f, SymbolNumber k)
{
    int byte;
    char * sym = line;
    while ( (byte = fgetc(f)) != 0 )
    {
	if (byte == EOF)
	{
	    throw AlphabetParsingException();
	}
	*sym = byte;
	++sym;
    }
    if (k == 0) {
	kt->push_back(std::string(""));
	return; // ignore epsilon
    }
    *sym = 0;

    // Now we detect and handle special symbols, which begin and end with @
    if (line[0] == '@' && line[strlen(line) - 1] == '@') {
	if (strlen(line) >= 5 && line[2] == '.') { // flag diacritic
	    std::string feat;
	    std::string val;
	    FlagDiacriticOperator op = P; // for the compiler
	    switch (line[1]) {
	    case 'P': op = P; break;
	    case 'N': op = N; break;
	    case 'R': op = R; break;
	    case 'D': op = D; break;
	    case 'C': op = C; break;
	    case 'U': op = U; break;
	    }
	    char * c = line;
	    for (c +=3; *c != '.' && *c != '@'; c++) { feat.append(c,1); }
	    if (*c == '.')
	    {
		for (++c; *c != '@'; c++) { val.append(c,1); }
	    }
	    if (feature_bucket.count(feat) == 0)
	    {
		feature_bucket[feat] = feat_num;
		++feat_num;
	    }
	    if (value_bucket.count(val) == 0)
	    {
		value_bucket[val] = val_num;
		++val_num;
	    }
	  
	    operations.insert(
		std::pair<SymbolNumber, FlagDiacriticOperation>(
		    k,
		    FlagDiacriticOperation(
			op, feature_bucket[feat], value_bucket[val])));
	  
	    kt->push_back(std::string(""));
	    return;
	  
	} else if (strlen(line) == 3 and line[1] == '?') { // other symbol
	    other_symbol = k;
	    kt->push_back(std::string(""));
	    return;
	} else { // we don't know what this is, ignore and suppress
	    kt->push_back(std::string(""));
	    return;
	}
    }
    kt->push_back(std::string(line));
    string_to_symbol[std::string(line)] = k;
}

void IndexTableReader::get_index_vector(void)
{
    for (size_t i = 0;
	 i < number_of_table_entries;
	 ++i)
    {
	size_t j = i * TransitionIndex::SIZE;
	SymbolNumber * input = (SymbolNumber*)(TableIndices + j);
	TransitionTableIndex * index = 
	    (TransitionTableIndex*)(TableIndices + j + sizeof(SymbolNumber));
	indices.push_back(new TransitionIndex(*input,*index));
    }
}

void TransitionTableReader::get_transition_vector(void)
{
    for (size_t i = 0; i < number_of_table_entries; ++i)
    {
	size_t j = i * Transition::SIZE;
	SymbolNumber * input = (SymbolNumber*)(TableTransitions + j);
	SymbolNumber * output = 
	    (SymbolNumber*)(TableTransitions + j + sizeof(SymbolNumber));
	TransitionTableIndex * target = 
	    (TransitionTableIndex*)(TableTransitions + j + 2 * sizeof(SymbolNumber));
	Weight * weight =
	    (Weight*)(TableTransitions + j + 2 * sizeof(SymbolNumber) +
		      sizeof(TransitionTableIndex));
      
	transitions.push_back(new Transition(*input,
					     *output,
					     *target,
					     *weight));
      
    }
}

void LetterTrie::add_string(const char * p, SymbolNumber symbol_key)
{
    if (*(p+1) == 0)
    {
	symbols[(unsigned char)(*p)] = symbol_key;
	return;
    }
    if (letters[(unsigned char)(*p)] == NULL)
    {
	letters[(unsigned char)(*p)] = new LetterTrie();
    }
    letters[(unsigned char)(*p)]->add_string(p+1,symbol_key);
}

SymbolNumber LetterTrie::find_key(char ** p)
{
    const char * old_p = *p;
    ++(*p);
    if (letters[(unsigned char)(*old_p)] == NULL)
    {
	return symbols[(unsigned char)(*old_p)];
    }
    SymbolNumber s = letters[(unsigned char)(*old_p)]->find_key(p);
    if (s == NO_SYMBOL)
    {
	--(*p);
	return symbols[(unsigned char)(*old_p)];
    }
    return s;
}

void Encoder::read_input_symbols(KeyTable * kt)
{
    for (SymbolNumber k = 0; k < number_of_input_symbols; ++k)
    {
	const char * p = kt->at(k).c_str();
	if (strlen(p) == 0) { // ignore empty strings
	    continue;
	}
	if ((strlen(p) == 1) && (unsigned char)(*p) <= 127)
	{
	    ascii_symbols[(unsigned char)(*p)] = k;
	}
	letters.add_string(p,k);
    }
}

SymbolNumber Encoder::find_key(char ** p)
{
    if (ascii_symbols[(unsigned char)(**p)] == NO_SYMBOL)
    {
	return letters.find_key(p);
    }
    SymbolNumber s = ascii_symbols[(unsigned char)(**p)];
    ++(*p);
    return s;
}

} // namespace hfst_ol
