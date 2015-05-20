/* -*- Mode: C++ -*- */
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

#ifndef HFST_OSPELL_OSPELL_H_
#define HFST_OSPELL_OSPELL_H_ 1

#include <string>
#include <deque>
#include <queue>
#include <stdexcept>
#include <limits>
#include "hfst-ol.h"

namespace hfst_ol {

//! Internal class for transition processing.

class TreeNode;
class CacheContainer;
typedef std::pair<std::string, std::string> StringPair;
typedef std::pair<std::string, Weight> StringWeightPair;
typedef std::vector<StringWeightPair> StringWeightVector;
typedef std::pair<std::pair<std::string, std::string>, Weight>
                                                        StringPairWeightPair;
typedef std::vector<TreeNode> TreeNodeVector;
typedef std::map<std::string, Weight> StringWeightMap;

//! Contains low-level processing stuff.
class STransition{
public:
    TransitionTableIndex index; //!< index to transition
    SymbolNumber symbol; //!< symbol of transition
    Weight weight; //!< wieght of transtios

    //!
    //! create transition without weight
    STransition(TransitionTableIndex i,
                SymbolNumber s):
        index(i),
        symbol(s),
        weight(0.0)
        {}

    //! create transition with weight
    STransition(TransitionTableIndex i,
                SymbolNumber s,
                Weight w):
        index(i),
        symbol(s),
        weight(w)
        {}

};
//! @brief comparison for establishing order for priority queue for suggestions.

//! The suggestions that are stored in a priority queue are arranged in
//! ascending order of the weight component, following the basic penalty
//! weight logic of tropical semiring that is present in most weighted
//! finite-state spell-checking automata.
class StringWeightComparison
/* results are reversed by default because greater weights represent
   worse results - to reverse the reversal, give a true argument*/

{
    bool reverse;
public:
    //!
    //! construct a result comparator with ascending or descending weight order
    StringWeightComparison(bool reverse_result=false):
        reverse(reverse_result)
        {}
    
    //!
    //! compare two string weight pairs for weights
    bool operator() (StringWeightPair lhs, StringWeightPair rhs);
};

//! @brief comparison for complex analysis queues
//
//! Follows weight value logic.
//! @see StringWeightComparison.
class StringPairWeightComparison
{
    bool reverse;
public:
    //!
    //! create result comparator with ascending or descending weight order
    StringPairWeightComparison(bool reverse_result=false):
        reverse(reverse_result)
        {}
    
    //!
    //! compare two analysis corrections for weights
    bool operator() (StringPairWeightPair lhs, StringPairWeightPair rhs);
};

typedef std::priority_queue<StringWeightPair,
                            std::vector<StringWeightPair>,
                            StringWeightComparison> CorrectionQueue;
typedef std::priority_queue<StringWeightPair,
                            std::vector<StringWeightPair>,
                            StringWeightComparison> AnalysisQueue;
typedef std::priority_queue<StringWeightPair,
                            std::vector<StringWeightPair>,
                            StringWeightComparison> HyphenationQueue;
typedef std::priority_queue<StringPairWeightPair,
                            std::vector<StringPairWeightPair>,
                            StringPairWeightComparison> AnalysisCorrectionQueue;
typedef std::priority_queue<Weight> WeightQueue;

//! Internal class for Transducer processing.

//! Contains low-level processing stuff.
class Transducer
{
protected:
    TransducerHeader header; //!< header data
    TransducerAlphabet alphabet; //!< alphabet data
    KeyTable * keys; //!< key symbol mappings
    Encoder encoder; //!< encoder to convert the strings

    static const TransitionTableIndex START_INDEX = 0; //!< position of first
  
    std::vector<const char*> symbol_table; //!< symbols known
 
    //!
    //! set symbol table to void
    void set_symbol_table(void);

 
public:
    //! 
    //! read transducer from file @a f
    Transducer(FILE * f);
    //!
    //! read transducer from raw dara @a data
    Transducer(char * raw);
    //!
    //! Analyse string @a line using the transducer
    AnalysisQueue lookup(char * line);
    IndexTable indices; //!< index table
    TransitionTable transitions; //!< transition table
    //!
    //! whether it's final transition in this transducer
    bool final_transition(TransitionTableIndex i);
    //!
    //! whether it's final index 
    bool final_index(TransitionTableIndex i);
    //!
    //! get transducers symbol table mapping
    KeyTable * get_key_table(void);
    //!
    //! find key for string or create it
    SymbolNumber find_next_key(char ** p);
    //!
    //! get encoder for mapping sttrings and symbols
    Encoder * get_encoder(void);
    //!
    //! get size of a state
    unsigned int get_state_size(void);
    //!
    //! get symbols of automatn
    std::vector<const char*> * get_symbol_table(void);
    //!
    //! get position of the other symbol
    SymbolNumber get_other(void);
    //!
    //! get alphabet of automaton
    TransducerAlphabet * get_alphabet(void);
    //!
    //! get flag stuff of automaton
    OperationMap * get_operations(void);
    //!
    //! follow epsilon transitions from index
    STransition take_epsilons(const TransitionTableIndex i) const;
    //!
    //! follow epsilon transitions and falsg form index
    STransition take_epsilons_and_flags(const TransitionTableIndex i);
    //! 
    //! follow real transitions from index
    STransition take_non_epsilons(const TransitionTableIndex i,
                                  const SymbolNumber symbol) const;
    //!
    //! get next index
    TransitionTableIndex next(const TransitionTableIndex i,
                              const SymbolNumber symbol) const;
    //!
    //! get next epsilon inedx
    TransitionTableIndex next_e(const TransitionTableIndex i) const;
    //! 
    //! whether state has any transitions with @a symbol
    bool has_transitions(const TransitionTableIndex i,
                         const SymbolNumber symbol) const;
    //!
    //! whether state has epsilon s or flag s
    bool has_epsilons_or_flags(const TransitionTableIndex i);
    //!
    //! whether state has non-epsilons or non-flags
    bool has_non_epsilons_or_flags(const TransitionTableIndex i);
    //! 
    //! whether it's final
    bool is_final(const TransitionTableIndex i);
    //! 
    //! get final weight
    Weight final_weight(const TransitionTableIndex i) const;
    //!
    //! whether it's a flag
    bool is_flag(const SymbolNumber symbol);
    //!
    //! whether it's weighedc
    bool is_weighted(void);

};

//! Internal class for alphabet processing.

//! Contains low-level processing stuff.
struct TreeNode
{
//    SymbolVector input_string; //<! the current input vector
    SymbolVector string; //!< the current output vector
    unsigned int input_state; //!< its input state
    TransitionTableIndex mutator_state; //!< state in error model
    TransitionTableIndex lexicon_state; //!< state in language model
    FlagDiacriticState flag_state; //!< state of flags
    Weight weight; //!< weight

    //!
    //! construct a node in trie from all that stuff
    TreeNode(SymbolVector prev_string,
             unsigned int i,
             TransitionTableIndex mutator,
             TransitionTableIndex lexicon,
             FlagDiacriticState state,
             Weight w):
        string(prev_string),
        input_state(i),
        mutator_state(mutator),
        lexicon_state(lexicon),
        flag_state(state),
        weight(w)
        { }

    //! 
    //! construct empty node with a starting state for flags
    TreeNode(FlagDiacriticState start_state): // starting state node
    string(SymbolVector()),
    input_state(0),
    mutator_state(0),
    lexicon_state(0),
    flag_state(start_state),
    weight(0.0)
        { }

    //!
    //! check if tree node is compatible with flag diacritc
    bool try_compatible_with(FlagDiacriticOperation op);

    //!
    //! traverse some node in lexicon
    TreeNode update_lexicon(SymbolNumber next_symbol,
                            TransitionTableIndex next_lexicon,
                            Weight weight);

    //!
    //! traverse some node in error model
    TreeNode update_mutator(SymbolNumber next_symbol,
                            TransitionTableIndex next_mutator,
                            Weight weight);

    //!
    //! do some stuff with error model
    void increment_mutator(void);

    //!
    //! The update functions return updated copies of this state
    TreeNode update(SymbolNumber output_symbol,
                    unsigned int next_input,
                    TransitionTableIndex next_mutator,
                    TransitionTableIndex next_lexicon,
                    Weight weight);

    TreeNode update(SymbolNumber output_symbol,
                    TransitionTableIndex next_mutator,
                    TransitionTableIndex next_lexicon,
                    Weight weight);


};

typedef std::vector<TreeNode> TreeNodeQueue;

int nByte_utf8(unsigned char c);

//! Internal class for string processing.

//! Contains low-level processing stuff.
class InputString
{
  
private:
    SymbolVector s;

public:
    InputString():
        s(SymbolVector())
        { }

    //!
    //! set up string with encoder and input and other symbol
    bool initialize(Encoder * encoder, char * input, SymbolNumber other);
    
    //!
    //! lengtho fo teh setring
    unsigned int len(void);

    //!
    //! character at position
    SymbolNumber operator[](unsigned int i)
        {
            return s[i];
        }

};

//! Exception when speller cannot map characters of error model to language
//! model.

//! May get raised if error model automaton has output characters that are not
//! present in language model.
class AlphabetTranslationException: public std::runtime_error
{ // "what" should hold the first untranslatable symbol
public:
    
    //!
    //! create alpabet exception with symbol as explanation
    AlphabetTranslationException(const std::string what):
        std::runtime_error(what)
        { }
};

//! @brief Basic spell-checking automata pair unit.

//! Speller consists of two automata, one for language modeling and one for
//! error modeling. The speller object has low-level access to the automata
//! and convenience functions for checking, analysing and correction.
//! @see ZHfstOspeller for high level access.
class Speller
{
public:
    Transducer * mutator; //!< error model
    Transducer * lexicon; //!< languag model
    InputString input; //!< current input
    TreeNodeQueue queue; //!< current traversal fifo stack
    TreeNode next_node;  //!< current next node
    Weight limit; //!< current limit for weights
    Weight best_suggestion; //!< best suggestion so far
    WeightQueue nbest_queue; //!< queue to keep track of current n best results
    SymbolVector alphabet_translator; //!< alphabets in automata
    OperationMap * operations; //!< flags in it
    std::vector<const char*> * symbol_table; //!< strings for symbols
    std::vector<CacheContainer> cache;
//!< A cache for the result of first symbols
    enum LimitingBehaviour { None, MaxWeight, Nbest, Beam, MaxWeightNbest,
                             MaxWeightBeam, NbestBeam, MaxWeightNbestBeam } limiting;
    //!< what kind of limiting behaviour we have

    
    //!
    //! Create a speller object form error model and language automata.
    Speller(Transducer * mutator_ptr, Transducer * lexicon_ptr);
    //!
    //! Create inpyt for speller
    bool init_input(char * str, Encoder * encoder, SymbolNumber other);
    //!
    //! size of states
    SymbolNumber get_state_size(void);
    //!
    //! initialise string conversions
    void build_alphabet_translator(void);
    //!
    //! travers epsilons in language model
    void lexicon_epsilons(void);
    //!
    //! traverse epsilons in error modle
    void mutator_epsilons(void);
    //!
    //! traverse along input
    void consume_input(SymbolNumber input_sym = NO_SYMBOL);
    //!
    //! follow language model with stuff
    void lexicon_consume(void);
    //! @brief Check if the given string is accepted by the speller
    //
    //! foo
    bool check(char * line);
    //! @brief suggest corrections for given string @a line.
    //
    //! The number of corrections given and stored at any given time
    //! is limited by @a nbest if ≥ 0. 
    CorrectionQueue correct(char * line, int nbest = 0,
                            Weight maxweight = -1.0,
                            Weight beam = -1.0);

    void set_limiting_behaviour(int nbest, Weight maxweight, Weight beam);
    void adjust_weight_limits(int nbest, Weight beam);
    
    //! @brief analyse given string @a line.
    //
    //! If language model is two-tape, give a list of analyses for string.
    //! If not, this should return queue of one result @a line if the
    //! string is in language model and 0 results if it isn't.
    AnalysisQueue analyse(char * line, int nbest = 0);

    void build_cache(SymbolNumber first_sym);
    //! @brief Construct a cache entry for @a first_sym..

};

struct CacheContainer
{
    // All the nodes that ultimately result from searching at input depth 1
    TreeNodeVector nodes;
    // The results are for length max one inputs only
    StringWeightVector results_len_0;
    StringWeightVector results_len_1;
    bool empty;

    CacheContainer(void): empty(true) {}
    
    void clear(void)
        {
            nodes.clear();
            results_len_0.clear();
            results_len_1.clear();
        }
    
};

std::string stringify(std::vector<const char *> * symbol_table,
                      SymbolVector & symbol_vector);


} // namespace hfst_ol

// Some platforms lack strndup
char* hfst_strndup(const char* s, size_t n);
    
#endif // HFST_OSPELL_OSPELL_H_
