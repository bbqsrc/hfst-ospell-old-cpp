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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ospell.h"

namespace hfst_ol {

int nByte_utf8(unsigned char c)
{
    /* utility function to determine how many bytes to peel off as
       a utf-8 character for representing as OTHER */
    if (c <= 127) {
        return 1;
    } else if ( (c & (128 + 64 + 32 + 16)) == (128 + 64 + 32 + 16) ) {
        return 4;
    } else if ( (c & (128 + 64 + 32 )) == (128 + 64 + 32) ) {
        return 3;
    } else if ( (c & (128 + 64 )) == (128 + 64)) {
        return 2;
    } else {
        return 0;
    }
}

bool
StringWeightComparison::operator()(StringWeightPair lhs, StringWeightPair rhs)
{ // return true when we want rhs to appear before lhs
    if (reverse) {
        return (lhs.second < rhs.second);
    } else {
        return (lhs.second > rhs.second);
    }
}

bool
StringPairWeightComparison::operator()(StringPairWeightPair lhs,
                                       StringPairWeightPair rhs)
{ // return true when we want rhs to appear before lhs
    if (reverse) {
        return (lhs.second < rhs.second);
    } else {
        return (lhs.second > rhs.second);
    }
}
Transducer::Transducer(FILE* f):
    header(TransducerHeader(f)),
    alphabet(TransducerAlphabet(f, header.symbol_count())),
    keys(alphabet.get_key_table()),
    encoder(keys,header.input_symbol_count()),
    indices(f,header.index_table_size()),
    transitions(f,header.target_table_size())
        {
            set_symbol_table();
        }

Transducer::Transducer(char* raw):
    header(TransducerHeader(&raw)),
    alphabet(TransducerAlphabet(&raw, header.symbol_count())),
    keys(alphabet.get_key_table()),
    encoder(keys,header.input_symbol_count()),
    indices(&raw,header.index_table_size()),
    transitions(&raw,header.target_table_size())
        {
            set_symbol_table();
        }


bool InputString::initialize(Encoder * encoder,
                             char * input,
                             SymbolNumber other)
{
    // Initialize the symbol vector to the tokenization given by encoder.
    // In the case of tokenization failure, valid utf-8 characters
    // are tokenized as "other" and tokenization is reattempted from
    // such a character onwards. The empty string is tokenized as an
    // empty vector; there is no end marker.
    
    s.clear();
    SymbolNumber k = NO_SYMBOL;
    char ** inpointer = &input;
    char * oldpointer;
    
    while (**inpointer != '\0') {
        oldpointer = *inpointer;
        k = encoder->find_key(inpointer);
    
        if (k == NO_SYMBOL) { // no tokenization from alphabet
            int n = nByte_utf8(static_cast<unsigned char>(*oldpointer));
            if (n == 0) {
                return false; // can't parse utf-8 character, admit failure
            } else {
                if (other == NO_SYMBOL) {
                    return false; // if we don't have an "other" symbol
                }
                oldpointer += n;
                *inpointer = oldpointer;
                s.push_back(other);
                continue;
            }
        } else {
            s.push_back(k);
        }
    }
    
    return true;
}

unsigned int
InputString::len()
{
    return s.size();
}


TreeNode TreeNode::update_lexicon(SymbolNumber output_symbol,
                                  TransitionTableIndex next_lexicon,
                                  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(output_symbol);
    return TreeNode(str,
                    this->input_state,
                    this->mutator_state,
                    next_lexicon,
                    this->flag_state,
                    this->weight + weight);
}

TreeNode TreeNode::update_mutator(SymbolNumber output_symbol,
                                  TransitionTableIndex next_mutator,
                                  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(output_symbol);
    return TreeNode(str,
                    this->input_state,
                    next_mutator,
                    this->lexicon_state,
                    this->flag_state,
                    this->weight + weight);
}


TreeNode TreeNode::update(SymbolNumber symbol,
                          unsigned int next_input,
                          TransitionTableIndex next_mutator,
                          TransitionTableIndex next_lexicon,
                          Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(symbol);
    return TreeNode(str,
                    next_input,
                    next_mutator,
                    next_lexicon,
                    this->flag_state,
                    this->weight + weight);
}

TreeNode TreeNode::update(SymbolNumber symbol,
                          TransitionTableIndex next_mutator,
                          TransitionTableIndex next_lexicon,
                          Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(symbol);
    return TreeNode(str,
                    this->input_state,
                    next_mutator,
                    next_lexicon,
                    this->flag_state,
                    this->weight + weight);
}

bool TreeNode::try_compatible_with(FlagDiacriticOperation op)
{
    switch (op.Operation()) {
    
    case P: // positive set
        flag_state[op.Feature()] = op.Value();
        return true;
    
    case N: // negative set (literally, in this implementation)
        flag_state[op.Feature()] = -1*op.Value();
        return true;
    
    case R: // require
        if (op.Value() == 0) { // "plain" require, return false if unset
            return (flag_state[op.Feature()] != 0);
        }
        return (flag_state[op.Feature()] == op.Value());
    
    case D: // disallow
        if (op.Value() == 0) { // "plain" disallow, return true if unset
            return (flag_state[op.Feature()] == 0);
        }
        return (flag_state[op.Feature()] != op.Value());
      
    case C: // clear
        flag_state[op.Feature()] = 0;
        return true;
      
    case U: // unification
        /* if the feature is unset OR the feature is to this value already OR
           the feature is negatively set to something else than this value */
        if (flag_state[op.Feature()] == 0 ||
            flag_state[op.Feature()] == op.Value() ||
            (flag_state[op.Feature()] < 0 &&
             (flag_state[op.Feature()] * -1 != op.Value()))
            ) {
            flag_state[op.Feature()] = op.Value();
            return true;
        }
        return false;
    }
    
    return false; // to make the compiler happy
}

Speller::Speller(Transducer* mutator_ptr, Transducer* lexicon_ptr):
        mutator(mutator_ptr),
        lexicon(lexicon_ptr),
        input(InputString()),
        queue(TreeNodeQueue()),
        next_node(FlagDiacriticState(get_state_size(), 0)),
        alphabet_translator(SymbolVector()),
        operations(lexicon->get_operations()),
        symbol_table(lexicon->get_symbol_table())
        {
          if (mutator != 0)
            build_alphabet_translator();
        }


SymbolNumber
Speller::get_state_size()
{
    return lexicon->get_state_size();
}


void Speller::lexicon_epsilons(void)
{
    if (!lexicon->has_epsilons_or_flags(next_node.lexicon_state + 1)) {
        return;
    }
    TransitionTableIndex next = lexicon->next(next_node.lexicon_state, 0);
    STransition i_s = lexicon->take_epsilons_and_flags(next);
    
    while (i_s.symbol != NO_SYMBOL) {
        if (lexicon->transitions.input_symbol(next) == 0) {
            queue.push_back(next_node.update_lexicon(0, // Update with epsilon because we want
                                                        // the surface tape for correcting
                                                     i_s.index,
                                                     i_s.weight));
        } else {
            FlagDiacriticState old_flags = next_node.flag_state;
            if (next_node.try_compatible_with( // this is terrible
                    operations->operator[](
                        lexicon->transitions.input_symbol(next)))) {
                queue.push_back(next_node.update_lexicon(0,
                                                         i_s.index,
                                                         i_s.weight));
                next_node.flag_state = old_flags;
            }
        }
        ++next;
        i_s = lexicon->take_epsilons_and_flags(next);
    }
}

void Speller::lexicon_consume(void)
{
    unsigned int input_state = next_node.input_state;
    if (input_state >= input.len()||
        !lexicon->has_transitions(
            next_node.lexicon_state + 1, input[input_state])) {
        return;
    }

    TransitionTableIndex next = lexicon->next(next_node.lexicon_state,
                                              input[input_state]);
    STransition i_s = lexicon->take_non_epsilons(next,
                                                 input[input_state]);

    while (i_s.symbol != NO_SYMBOL) {
        queue.push_back(next_node.update(
                            i_s.symbol,
                            input_state + 1,
                            next_node.mutator_state,
                            i_s.index,
                            i_s.weight));
    
        ++next;
        i_s = lexicon->take_non_epsilons(next, input[input_state]);
    }
    
}

void Speller::mutator_epsilons(void)
{
    if (!mutator->has_transitions(next_node.mutator_state + 1, 0)) {
        return;
    }
    TransitionTableIndex next_m = mutator->next(next_node.mutator_state, 0);
    STransition mutator_i_s = mutator->take_epsilons(next_m);
   
    while (mutator_i_s.symbol != NO_SYMBOL) {
        if (mutator_i_s.symbol == 0) {
            queue.push_back(next_node.update_mutator(mutator_i_s.symbol,
                                                     mutator_i_s.index,
                                                     mutator_i_s.weight));
        } else {
            if (!lexicon->has_transitions(
                    next_node.lexicon_state + 1,
                    alphabet_translator[mutator_i_s.symbol])) {
                ++next_m;
                mutator_i_s = mutator->take_epsilons(next_m);
                continue;
            }
            TransitionTableIndex next_l = lexicon->next(
                next_node.lexicon_state,
                alphabet_translator[mutator_i_s.symbol]);
            STransition lexicon_i_s = lexicon->take_non_epsilons(
                next_l,
                alphabet_translator[mutator_i_s.symbol]);
        
            while (lexicon_i_s.symbol != NO_SYMBOL) {
                queue.push_back(next_node.update(
                                    alphabet_translator[mutator_i_s.symbol],
                                    mutator_i_s.index,
                                    lexicon_i_s.index,
                                    lexicon_i_s.weight + mutator_i_s.weight));
                ++next_l;
                lexicon_i_s = lexicon->take_non_epsilons(
                    next_l,
                    alphabet_translator[mutator_i_s.symbol]);
            }
        }
        ++next_m;
        mutator_i_s = mutator->take_epsilons(next_m);
    }
}

void Speller::consume_input(void)
{
    unsigned int input_state = next_node.input_state;
    if (input_state >= input.len()||
        !mutator->has_transitions(next_node.mutator_state + 1,
                                  input[input_state])) {
        return; // not enough input to consume or no suitable transitions
    }
    
    TransitionTableIndex next_m = mutator->next(next_node.mutator_state,
                                                input[input_state]);
    
    STransition mutator_i_s = mutator->take_non_epsilons(next_m,
                                                         input[input_state]);
    
    while (mutator_i_s.symbol != NO_SYMBOL) {

        if (mutator_i_s.symbol == 0) {
        
            queue.push_back(next_node.update(0,
                                             input_state + 1,
                                             mutator_i_s.index,
                                             next_node.lexicon_state,
                                             mutator_i_s.weight));
        } else {
            if (!lexicon->has_transitions(
                    next_node.lexicon_state + 1,
                    alphabet_translator[mutator_i_s.symbol])) {
                ++next_m;
                mutator_i_s = mutator->take_non_epsilons(next_m,
                                                         input[input_state]);
                continue;
            }
            TransitionTableIndex next_l = lexicon->next(
                next_node.lexicon_state,
                alphabet_translator[mutator_i_s.symbol]);
        
            STransition lexicon_i_s = lexicon->take_non_epsilons(
                next_l,
                alphabet_translator[mutator_i_s.symbol]);
        
            while (lexicon_i_s.symbol != NO_SYMBOL) {
                queue.push_back(
                    next_node.update(alphabet_translator[mutator_i_s.symbol],
                                     input_state + 1,
                                     mutator_i_s.index,
                                     lexicon_i_s.index,
                                     lexicon_i_s.weight + mutator_i_s.weight));
                ++next_l;
                lexicon_i_s = lexicon->take_non_epsilons(
                    next_l,
                    alphabet_translator[mutator_i_s.symbol]);
            }
        }
        ++next_m;
        mutator_i_s = mutator->take_non_epsilons(next_m,
                                                 input[input_state]);
    }
}

bool
Transducer::final_transition(TransitionTableIndex i)
{
    return transitions.final(i);
}

bool
Transducer::final_index(TransitionTableIndex i)
{
    return indices.final(i);
}

KeyTable*
Transducer::get_key_table()
{
    return keys;
}

SymbolNumber
Transducer::find_next_key(char** p)
{
    return encoder.find_key(p);
}

Encoder*
Transducer::get_encoder()
{
    return &encoder;
}

unsigned int
Transducer::get_state_size()
{
    return alphabet.get_state_size();
}

std::vector<const char*>*
Transducer::get_symbol_table()
{
    return &symbol_table;
}

SymbolNumber
Transducer::get_other()
{
    return alphabet.get_other();
}

TransducerAlphabet*
Transducer::get_alphabet()
{
    return &alphabet;
}

OperationMap*
Transducer::get_operations()
{
    return alphabet.get_operation_map();
}

TransitionTableIndex Transducer::next(const TransitionTableIndex i,
                                      const SymbolNumber symbol) const
{
    if (i >= TARGET_TABLE) {
        return i - TARGET_TABLE + 1;
    } else {
        return indices.target(i+1+symbol) - TARGET_TABLE;
    }
}

bool Transducer::has_transitions(const TransitionTableIndex i,
                                 const SymbolNumber symbol) const
{
    if (symbol == NO_SYMBOL) {
        return false;
    }
    if (i >= TARGET_TABLE) {
        return (transitions.input_symbol(i - TARGET_TABLE) == symbol);
    } else {
        return (indices.input_symbol(i+symbol) == symbol);
    }
}

bool Transducer::has_epsilons_or_flags(const TransitionTableIndex i)
{
    if (i >= TARGET_TABLE) {
        return(transitions.input_symbol(i - TARGET_TABLE) == 0||
               is_flag(transitions.input_symbol(i - TARGET_TABLE)));
    } else {
        return (indices.input_symbol(i) == 0);
    }
}

STransition Transducer::take_epsilons(const TransitionTableIndex i) const
{
    if (transitions.input_symbol(i) != 0) {
        return STransition(0, NO_SYMBOL);
    }
    return STransition(transitions.target(i),
                       transitions.output_symbol(i),
                       transitions.weight(i));
}

STransition Transducer::take_epsilons_and_flags(const TransitionTableIndex i)
{
    if (transitions.input_symbol(i) != 0 &&
        !is_flag(transitions.input_symbol(i))) {
        return STransition(0, NO_SYMBOL);
    }
    return STransition(transitions.target(i),
                       transitions.output_symbol(i),
                       transitions.weight(i));
}

STransition Transducer::take_non_epsilons(const TransitionTableIndex i,
                                          const SymbolNumber symbol) const
{
    if (transitions.input_symbol(i) != symbol) {
        return STransition(0, NO_SYMBOL);
    }
    return STransition(transitions.target(i),
                       transitions.output_symbol(i),
                       transitions.weight(i));
}

bool Transducer::is_final(const TransitionTableIndex i)
{
    if (i >= TARGET_TABLE) {
        return final_transition(i - TARGET_TABLE);
    } else {
        return final_index(i);
    }
}

Weight Transducer::final_weight(const TransitionTableIndex i) const
{
    if (i >= TARGET_TABLE) {
        return transitions.weight(i - TARGET_TABLE);
    } else {
        return indices.final_weight(i);
    }
}

void Transducer::set_symbol_table(void)
{
    for(KeyTable::iterator it = keys->begin();
        it != keys->end();
        ++it)
    {
        const char * key_name = it->c_str();
        symbol_table.push_back(key_name);
    }
}

bool
Transducer::is_flag(const SymbolNumber symbol)
{
    return alphabet.is_flag(symbol); 
}

bool
Transducer::is_weighted(void)
{
    return header.probe_flag(Weighted);
}


AnalysisQueue Speller::analyse(char * line, int /* nbest */)
  {
    return lexicon->lookup(line);
  }

CorrectionQueue Speller::correct(char * line, int nbest)
{
    // if input initialization fails, return empty correction queue
    if (!init_input(line, mutator->get_encoder(), mutator->get_other())) {
        return CorrectionQueue();
    }
    std::map<std::string, Weight> corrections;
    TreeNode start_node(FlagDiacriticState(get_state_size(), 0));
    queue.assign(1, start_node);

    // A queue to keep track of the current n best results
    WeightQueue nbest_queue;

    while (queue.size() > 0) {
        /*
          For depth-first searching, we save the back node now, remove it
          from the queue and add new nodes to the search at the back.
         */
        next_node = queue.back();
        queue.pop_back();

        // if we can't get an acceptable result, never mind
        if (nbest > 0 && nbest_queue.size() > nbest &&
            next_node.weight >= nbest_queue.top()) {
            continue;
        }
        lexicon_epsilons();
        mutator_epsilons();
        if (next_node.input_state == input.len()) {
            /* if our transducers are in final states
             * we generate the correction
             */
            if (mutator->is_final(next_node.mutator_state)&&
                lexicon->is_final(next_node.lexicon_state)) {
                Weight weight = next_node.weight +
                    lexicon->final_weight(next_node.lexicon_state) +
                    mutator->final_weight(next_node.mutator_state);
                if (nbest > 0 && nbest_queue.size() > nbest &&
                    weight >= nbest_queue.top()) {
                    continue;
                }
                std::string string = stringify(symbol_table, next_node.string);
                /* if the correction is novel or better than before, insert it
                 */
                if (corrections.count(string) == 0||
                    corrections[string] > weight) {
                    corrections[string] = weight;
                    nbest_queue.push(weight);
                    if (nbest > 0 && nbest_queue.size() > nbest) {
                        nbest_queue.pop();
                    }
                }
            }
        } else {
            consume_input();
        }
    }
    CorrectionQueue correction_queue;
    std::map<std::string, Weight>::iterator it;
    for (it = corrections.begin(); it != corrections.end(); ++it) {
        correction_queue.push(StringWeightPair(it->first, it->second));
    }
    return correction_queue;
}

bool Speller::check(char * line)
{
    if (!init_input(line, lexicon->get_encoder(), NO_SYMBOL)) {
        return false;
    }
    TreeNode start_node(FlagDiacriticState(get_state_size(), 0));
    queue.assign(1, start_node);

    while (queue.size() > 0) {
        next_node = queue.back();
        queue.pop_back();
        if (next_node.input_state == input.len() &&
            lexicon->is_final(next_node.lexicon_state)) {
            return true;
        }
        lexicon_epsilons();
        lexicon_consume();
    }
    return false;
}

AnalysisQueue Transducer::lookup(char * line)
{
    std::map<std::string, Weight> outputs;
    AnalysisQueue analyses;
    InputString input;
    TreeNodeQueue queue;
    if (!input.initialize(&encoder, line, NO_SYMBOL)) {
        return analyses;
    }
    TreeNode start_node(FlagDiacriticState(get_state_size(), 0));
    queue.assign(1, start_node);

    while (queue.size() > 0) {
        TreeNode next_node = queue.back();
        queue.pop_back();

        // Final states
        if (next_node.input_state == input.len() &&
            is_final(next_node.lexicon_state)) {
            Weight weight = next_node.weight +
                final_weight(next_node.lexicon_state);
            std::string output = stringify(&symbol_table,
                                           next_node.string);
            /* if the result is novel or lower weighted than before, insert it */
            if (outputs.count(output) == 0 ||
                outputs[output] > weight) {
                outputs[output] = weight;
            }
        }

        TransitionTableIndex next_index;
        // epsilon loop
        if (has_epsilons_or_flags(next_node.lexicon_state + 1)) {
            next_index = next(next_node.lexicon_state, 0);
            STransition i_s = take_epsilons_and_flags(next_index);
            while (i_s.symbol != NO_SYMBOL) {
                if (transitions.input_symbol(next_index) == 0) {
                    queue.push_back(next_node.update_lexicon(i_s.symbol,
                                                             i_s.index,
                                                             i_s.weight));
                    // Not a true epsilon but a flag diacritic
                } else {
                    FlagDiacriticState old_flags = next_node.flag_state;
                    if (next_node.try_compatible_with(
                            get_operations()->operator[](
                                transitions.input_symbol(next_index)))) {
                        queue.push_back(next_node.update_lexicon(i_s.symbol,
                                                                 i_s.index,
                                                                 i_s.weight));
                        next_node.flag_state = old_flags;
                    }
                }
                ++next_index;
                i_s = take_epsilons_and_flags(next_index);
            }
        }
        
        // input consumption loop
        unsigned int input_state = next_node.input_state;
        if (input_state < input.len() &&
            has_transitions(
                next_node.lexicon_state + 1, input[input_state])) {
            
            next_index = next(next_node.lexicon_state,
                              input[input_state]);
            STransition i_s = take_non_epsilons(next_index,
                                                input[input_state]);
            
            while (i_s.symbol != NO_SYMBOL) {
                queue.push_back(next_node.update(
                                    i_s.symbol,
                                    input_state + 1,
                                    next_node.mutator_state,
                                    i_s.index,
                                    i_s.weight));
                
                ++next_index;
                i_s = take_non_epsilons(next_index, input[input_state]);
            }
        }
        
    }
    
    std::map<std::string, Weight>::const_iterator it;
    for (it = outputs.begin(); it != outputs.end(); ++it) {
        analyses.push(StringWeightPair(it->first, it->second));
    }
    
    return analyses;
}

std::string stringify(std::vector<const char *> * symbol_table,
                      SymbolVector & symbol_vector)
{
    std::string s;
    for (SymbolVector::iterator it = symbol_vector.begin();
         it != symbol_vector.end(); ++it) {
        s.append(symbol_table->at(*it));
    }
    return s;
}

bool Speller::init_input(char * str,
                         Encoder * encoder,
                         SymbolNumber other)
{
    return input.initialize(encoder, str, other);
}

void Speller::build_alphabet_translator(void)
{
    TransducerAlphabet * from = mutator->get_alphabet();
    TransducerAlphabet * to = lexicon->get_alphabet();
    KeyTable * from_keys = from->get_key_table();
    StringSymbolMap * to_symbols = to->get_string_to_symbol();
    alphabet_translator.push_back(0); // zeroth element is always epsilon
    for (SymbolNumber i = 1; i < from_keys->size(); ++i) {
        if ( (from->is_flag(i)) || // if it's a flag
             (i == from->get_other()) ) {   // or the OTHER symbol
            alphabet_translator.push_back(NO_SYMBOL);
            continue; // no translation
        }
        if (to_symbols->count(from_keys->operator[](i)) != 1) {
            // A symbol in the error source isn't present in the
            // lexicon. We can either error out or silently translate it to
            // NO_SYMBOL which will never be accepted by the lexicon.
            alphabet_translator.push_back(NO_SYMBOL);
            std::string name(from_keys->operator[](i));
            if (name != "") {
                std::cerr << "ospell: warning: symbol " <<
                    std::string(from_keys->operator[](i))
                          << " not present in lexicon\n";
            }
            continue;
            // If we want to die instead
                // throw AlphabetTranslationException(
                //     std::string(from_keys->operator[](i)));
        }
        // translator at i points to lexicon's symbol for mutator's string for
        // mutator's symbol number i
        alphabet_translator.push_back(
            to_symbols->operator[](
                from_keys->operator[](i)));
    }
}

} // namespace hfst_ol
  
char*
hfst_strndup(const char* s, size_t n)
  {
    char* rv = static_cast<char*>(malloc(sizeof(char)*n+1));
    if (rv == NULL)
      {
          return rv;
      }
    rv = static_cast<char*>(memcpy(rv, s, n));
    if (rv == NULL)
      {
        return rv;
      }
    rv[n] = '\0';
    return rv;
  }
    
