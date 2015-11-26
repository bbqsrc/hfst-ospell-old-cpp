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

#include "hfst-ol.h"
#include <string>

namespace hfst_ol {

Weight hfst_deref(const void* ptr) {
    Weight dest;
    memcpy(&dest, ptr, 4);
    return dest;
}

void skip_c_string(int8_t ** raw)
{
    while (**raw != 0)
    {
        ++(*raw);
    }
    ++(*raw);
}

void
TransducerHeader::read_property(bool& property, int8_t** raw)
{
    uint32_t prop = *((uint32_t *) *raw);
    (*raw) += sizeof(uint32_t);
    if (prop == 0)
    {
        property = false;
        return;
    }
    else
    {
        property = true;
        return;
    }
}

void TransducerHeader::skip_hfst3_header(int8_t ** raw)
{
    const char* header1 = "HFST";
    uint32_t header_loc = 0; // how much of the header has been found

    for(header_loc = 0; header_loc < strlen(header1) + 1; header_loc++)
    {
        if(**raw != header1[header_loc])
        {
            break;
        }
        ++(*raw);
    }
    if(header_loc == strlen(header1) + 1) // we found it
    {
        uint16_t remaining_header_len = *((uint16_t *) *raw);
        (*raw) += sizeof(uint16_t) + 1 + remaining_header_len;
    }
    else // nope. put back what we've taken
    {
        --(*raw); // first the non-matching character
        for(int32_t i = header_loc - 1; i>=0; i--)
        {
            // then the characters that did match (if any)
            --(*raw);
        }
    }
}

TransducerHeader::TransducerHeader(int8_t** raw)
{
    skip_hfst3_header(raw); // skip header iff it is present
    number_of_input_symbols = *(SymbolNumber*) *raw;
    (*raw) += sizeof(SymbolNumber);
    number_of_symbols = *(SymbolNumber*) *raw;
    (*raw) += sizeof(SymbolNumber);
    size_of_transition_index_table = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    size_of_transition_target_table = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    number_of_states = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    number_of_transitions = *(TransitionTableIndex*) *raw;
    (*raw) += sizeof(TransitionTableIndex);
    read_property(weighted,raw);
    read_property(deterministic,raw);
    read_property(input_deterministic,raw);
    read_property(minimized,raw);
    read_property(cyclic,raw);
    read_property(has_epsilon_epsilon_transitions,raw);
    read_property(has_input_epsilon_transitions,raw);
    read_property(has_input_epsilon_cycles,raw);
    read_property(has_unweighted_input_epsilon_cycles,raw);
}

SymbolNumber
TransducerHeader::symbol_count()
{
    return number_of_symbols;
}

SymbolNumber
TransducerHeader::input_symbol_count()
{
    return number_of_input_symbols;
}
TransitionTableIndex
TransducerHeader::index_table_size(void)
{
    return size_of_transition_index_table;
}

TransitionTableIndex
TransducerHeader::target_table_size()
{
    return size_of_transition_target_table;
}

bool
TransducerHeader::probe_flag(HeaderFlag flag)
{
    switch (flag)
    {
    case Weighted:
        return weighted;
    case Deterministic:
        return deterministic;
    case Input_deterministic:
        return input_deterministic;
    case Minimized:
        return minimized;
    case Cyclic:
        return cyclic;
    case Has_epsilon_epsilon_transitions:
        return has_epsilon_epsilon_transitions;
    case Has_input_epsilon_transitions:
        return has_input_epsilon_transitions;
    case Has_input_epsilon_cycles:
        return has_input_epsilon_cycles;
    case Has_unweighted_input_epsilon_cycles:
        return has_unweighted_input_epsilon_cycles;
    }
    return false;
}

bool
FlagDiacriticOperation::isFlag() const
{
    return feature != NO_SYMBOL;
}

FlagDiacriticOperator
FlagDiacriticOperation::Operation() const
{
    return operation;
}

SymbolNumber
FlagDiacriticOperation::Feature() const
{
    return feature;
}


ValueNumber
FlagDiacriticOperation::Value() const
{
    return value;
}

void TransducerAlphabet::read(int8_t ** raw, SymbolNumber number_of_symbols)
{
    std::map<std::string, SymbolNumber> feature_bucket;
    std::map<std::string, ValueNumber> value_bucket;
    value_bucket[std::string()] = 0; // empty value = neutral
    ValueNumber val_num = 1;
    SymbolNumber feat_num = 0;

    kt.push_back(std::string("")); // zeroth symbol is epsilon
    skip_c_string(raw);

    for (SymbolNumber k = 1; k < number_of_symbols; ++k)
    {

        // Detect and handle special symbols, which begin and end with @
        if ((*raw)[0] == '@' && (*raw)[strlen((char*)*raw) - 1] == '@')
        {
            if (strlen((char*)*raw) >= 5 && (*raw)[2] == '.')   // flag diacritic
            {
                std::string feat;
                std::string val;
                FlagDiacriticOperator op = P; // for the compiler
                switch ((*raw)[1])
                {
                case 'P': op = P; break;
                case 'N': op = N; break;
                case 'R': op = R; break;
                case 'D': op = D; break;
                case 'C': op = C; break;
                case 'U': op = U; break;
                }
                int8_t * c = *raw;
                for (c += 3; *c != '.' && *c != '@'; c++)
                {
                    feat.append((char*)c, 1);
                }
                if (*c == '.')
                {
                    for (++c; *c != '@'; c++)
                    {
                        val.append((char*)c, 1);
                    }
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

                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;

            }
            else if (strcmp((char*)*raw, "@_UNKNOWN_SYMBOL_@") == 0)
            {
                unknown_symbol = k;
                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;
            }
            else if (strcmp((char*)*raw, "@_IDENTITY_SYMBOL_@") == 0)
            {
                identity_symbol = k;
                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;
            }
            else // we don't know what this is, ignore and suppress
            {
                kt.push_back(std::string(""));
                skip_c_string(raw);
                continue;
            }
        }
        kt.push_back(std::string((char*)*raw));
        string_to_symbol[std::string((char*)*raw)] = k;
        skip_c_string(raw);
    }
    flag_state_size = feature_bucket.size();
}

TransducerAlphabet::TransducerAlphabet(int8_t** raw,
                                       SymbolNumber number_of_symbols) :
    unknown_symbol(NO_SYMBOL),
    identity_symbol(NO_SYMBOL),
    orig_symbol_count(number_of_symbols)
{
    read(raw, number_of_symbols);
}

void TransducerAlphabet::add_symbol(std::string & sym)
{
    string_to_symbol[sym] = kt.size();
    kt.push_back(sym);
}

void TransducerAlphabet::add_symbol(int8_t * sym)
{
    std::string s((char*)sym);
    add_symbol(s);
}

KeyTable*
TransducerAlphabet::get_key_table()
{
    return &kt;
}

OperationMap*
TransducerAlphabet::get_operation_map()
{
    return &operations;
}

SymbolNumber
TransducerAlphabet::get_state_size()
{
    return flag_state_size;
}

SymbolNumber
TransducerAlphabet::get_unknown() const
{
    return unknown_symbol;
}

SymbolNumber
TransducerAlphabet::get_identity() const
{
    return identity_symbol;
}

SymbolNumber TransducerAlphabet::get_orig_symbol_count() const
{
    return orig_symbol_count;
}

StringSymbolMap*
TransducerAlphabet::get_string_to_symbol()
{
    return &string_to_symbol;
}

bool TransducerAlphabet::has_string(std::string const & s) const
{
    return string_to_symbol.count(s) != 0;
}

bool
TransducerAlphabet::is_flag(SymbolNumber symbol)
{
    return operations.count(symbol) == 1;
}

void IndexTable::read(int8_t ** raw,
                      TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*TransitionIndex::SIZE;
    //indices = (int8_t*)(malloc(table_size));
    //memcpy((void *) indices, (const void *) *raw, table_size);
    indices = *raw;
    (*raw) += table_size;
}

void TransitionTable::read(int8_t ** raw,
                           TransitionTableIndex number_of_table_entries)
{
    size_t table_size = number_of_table_entries*Transition::SIZE;
    //transitions = (int8_t*)(malloc(table_size));
    //memcpy((void *) transitions, (const void *) *raw, table_size);
    transitions = *raw;
    (*raw) += table_size;
}

void LetterTrie::add_string(const char* p, SymbolNumber symbol_key)
{
    if (*(p+1) == 0)
    {
        symbols[(uint8_t)(*p)] = symbol_key;
        return;
    }
    if (letters[(uint8_t)(*p)] == NULL)
    {
        letters[(uint8_t)(*p)] = new LetterTrie();
    }
    letters[(uint8_t)(*p)]->add_string(p+1,symbol_key);
}

SymbolNumber LetterTrie::find_key(int8_t ** p)
{
    const char * old_p = (char*)*p;
    ++(*p);
    if (letters[(uint8_t)(*old_p)] == NULL)
    {
        return symbols[(uint8_t)(*old_p)];
    }
    SymbolNumber s = letters[(uint8_t)(*old_p)]->find_key(p);
    if (s == NO_SYMBOL)
    {
        --(*p);
        return symbols[(uint8_t)(*old_p)];
    }
    return s;
}

LetterTrie::~LetterTrie()
{
    for (LetterTrieVector::iterator i = letters.begin();
         i != letters.end(); ++i)
    {
        if (*i)
        {
            delete *i;
        }
    }
}

Encoder::Encoder(KeyTable * kt, SymbolNumber number_of_input_symbols) :
    ascii_symbols(UCHAR_MAX,NO_SYMBOL)
{
    read_input_symbols(kt, number_of_input_symbols);
}

void Encoder::read_input_symbol(const char* s, const int32_t s_num)
{
    if (strlen(s) == 0)   // ignore empty strings
    {
        return;
    }
    if ((strlen(s) == 1) && (uint8_t)(*s) <= 127)
    {
        ascii_symbols[(uint8_t)(*s)] = s_num;
    }
    letters.add_string(s, s_num);
}

void Encoder::read_input_symbol(std::string const & s, const int32_t s_num)
{
    read_input_symbol(s.c_str(), s_num);
}

void Encoder::read_input_symbols(KeyTable * kt,
                                 SymbolNumber number_of_input_symbols)
{
    for (SymbolNumber k = 0; k < number_of_input_symbols; ++k)
    {
        const char * p = kt->at(k).c_str();
        read_input_symbol(p, k);
    }
}

TransitionTableIndex
TransitionIndex::target() const
{
    return first_transition_index;
}

bool
TransitionIndex::final (void) const
{
    return input_symbol == NO_SYMBOL &&
           first_transition_index != NO_TABLE_INDEX;
}

Weight
TransitionIndex::final_weight(void) const
{
    union to_weight
    {
        TransitionTableIndex i;
        Weight w;
    } weight;
    weight.i = first_transition_index;
    return weight.w;
}

SymbolNumber
TransitionIndex::get_input(void) const
{
    return input_symbol;
}

TransitionTableIndex
Transition::target(void) const
{
    return target_index;
}

SymbolNumber
Transition::get_output(void) const
{
    return output_symbol;
}

SymbolNumber
Transition::get_input(void) const
{
    return input_symbol;
}

Weight
Transition::get_weight(void) const
{
    return transition_weight;
}

bool
Transition::final (void) const
{
    return input_symbol == NO_SYMBOL &&
           output_symbol == NO_SYMBOL &&
           target_index == 1;
}

IndexTable::IndexTable(int8_t ** raw,
                       TransitionTableIndex number_of_table_entries) :
    indices(NULL),
    size(number_of_table_entries)
{
    read(raw, number_of_table_entries);
}

IndexTable::~IndexTable(void)
{
    /*if (indices)
    {
        free(indices);
    }*/
}

SymbolNumber
IndexTable::input_symbol(TransitionTableIndex i) const
{
    if (i < size)
    {
        return *((SymbolNumber *)
                 (indices + TransitionIndex::SIZE * i));
    }
    else
    {
        return NO_SYMBOL;
    }
}

TransitionTableIndex
IndexTable::target(TransitionTableIndex i) const
{
    if (i < size)
    {
        return *((TransitionTableIndex *)
                 (indices + TransitionIndex::SIZE * i +
                  sizeof(SymbolNumber)));
    }
    else
    {
        return NO_TABLE_INDEX;
    }
}

bool
IndexTable::final (TransitionTableIndex i) const
{
    return input_symbol(i) == NO_SYMBOL && target(i) != NO_TABLE_INDEX;
}

Weight
IndexTable::final_weight(TransitionTableIndex i) const
{
    if (i < size)
    {
        return hfst_deref((Weight *)
                          (indices + TransitionIndex::SIZE * i +
                           sizeof(SymbolNumber)));
    }
    else
    {
        return INFINITE_WEIGHT;
    }
}

TransitionTable::TransitionTable(int8_t ** raw,
                                 TransitionTableIndex transition_count) :
    transitions(NULL),
    size(transition_count)
{
    read(raw, transition_count);
}

TransitionTable::~TransitionTable(void)
{
    /*
    if (transitions)
    {
        free(transitions);
    }
    */
}

SymbolNumber
TransitionTable::input_symbol(TransitionTableIndex i) const
{
    if (i < size)
    {
        return *((SymbolNumber *)
                 (transitions + Transition::SIZE * i));
    }
    else
    {
        return NO_SYMBOL;
    }
}

SymbolNumber
TransitionTable::output_symbol(TransitionTableIndex i) const
{
    if (i < size)
    {
        return *((SymbolNumber *)
                 (transitions + Transition::SIZE * i +
                  sizeof(SymbolNumber)));
    }
    else
    {
        return NO_SYMBOL;
    }
}

TransitionTableIndex
TransitionTable::target(TransitionTableIndex i) const
{
    if (i < size)
    {
        return *((TransitionTableIndex *)
                 (transitions + Transition::SIZE * i +
                  2 * sizeof(SymbolNumber)));
    }
    else
    {
        return NO_TABLE_INDEX;
    }
}

Weight
TransitionTable::weight(TransitionTableIndex i) const
{
    if (i < size)
    {
        return hfst_deref((Weight*)
                          (transitions + Transition::SIZE * i +
                           2 * sizeof(SymbolNumber) +
                           sizeof(TransitionTableIndex)));
    }
    else
    {
        return INFINITE_WEIGHT;
    }
}

bool
TransitionTable::final (TransitionTableIndex i) const
{
    return input_symbol(i) == NO_SYMBOL &&
           output_symbol(i) == NO_SYMBOL &&
           target(i) == 1;
}

SymbolNumber Encoder::find_key(int8_t ** p)
{
    if (ascii_symbols[(uint8_t)(**p)] == NO_SYMBOL)
    {
        return letters.find_key(p);
    }
    SymbolNumber s = ascii_symbols[(uint8_t)(**p)];
    ++(*p);
    return s;
}

} // namespace hfst_ol
