/* -*- Mode: C++ -*- */

#ifndef HFST_OSPELL_OSPELL_H_
#define HFST_OSPELL_OSPELL_H_

#include <string>
#include <utility>
#include <getopt.h>
#include <deque>
#include <cassert>
#include "hfst-ol.h"

/* Just until we autotool */

#define PACKAGE_NAME "hfst-ospell"
#define PACKAGE_STRING "hfst-ospell beta"
#define PACKAGE_BUGREPORT "hfst-bugs@ling.helsinki.fi"
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define DEBUG 1

typedef std::pair<TransitionTableIndex, SymbolNumber> IndexSymbolPair;

class Transducer
{
protected:
    TransducerHeader header;
    TransducerAlphabet alphabet;
    KeyTable * keys;
    IndexTableReader index_reader;
    TransitionTableReader transition_reader;
    Encoder encoder;

    static const TransitionTableIndex START_INDEX = 0;
  
    std::vector<const char*> symbol_table;
  
    TransitionIndexVector &indices;
  
    TransitionVector &transitions;
  
    void set_symbol_table(void);

    bool final_transition(TransitionTableIndex i)
	{
	    return transitions[i]->final();
	}
    
    bool final_index(TransitionTableIndex i)
	{
	    return indices[i]->final();
	}
  
    TransitionTableIndex try_epsilon_indices(TransitionTableIndex i);
  
    IndexSymbolPair try_epsilon_transitions(TransitionTableIndex i);
  
    TransitionTableIndex find_index(SymbolNumber input,
				    TransitionTableIndex i);
  
    IndexSymbolPair find_transitions(SymbolNumber input,
				     TransitionTableIndex i);
  
public:
    Transducer(FILE * f):
	header(TransducerHeader(f)),
	alphabet(TransducerAlphabet(f, header.symbol_count())),
	keys(alphabet.get_key_table()),
	index_reader(f,header.index_table_size()),
	transition_reader(f,header.target_table_size()),
	encoder(keys,header.input_symbol_count()),
	indices(index_reader()),
	transitions(transition_reader())
	{
	    set_symbol_table();
	}
    
    KeyTable * get_key_table(void)
	{
	    return keys;
	}

    SymbolNumber find_next_key(char ** p)
	{
	    return encoder.find_key(p);
	}

    Encoder * get_encoder(void)
	{
	    return &encoder;
	}

    unsigned int get_state_size(void)
	{
	    return alphabet.get_state_size();
	}

    std::vector<const char*> * get_symbol_table(void)
	{
	    return &symbol_table;
	}

    SymbolNumber get_other(void)
	{
	    return alphabet.get_other();
	}

    TransducerAlphabet * get_alphabet(void)
	{
	    return &alphabet;
	}

    void free_temporary(void)
	{
	    alphabet.free_temporary();
	}

    IndexSymbolPair take_epsilons(TransitionTableIndex i);
    IndexSymbolPair take_non_epsilons(TransitionTableIndex i,
				      SymbolNumber symbol);
    bool is_final(TransitionTableIndex i);

};

class TreeNode
{
public:
    SymbolVector string;
    unsigned int input_state;
    TransitionTableIndex mutator_state;
    TransitionTableIndex lexicon_state;
    FlagDiacriticState flag_state;

    TreeNode(SymbolVector prev_string,
	     unsigned int i,
	     TransitionTableIndex mutator,
	     TransitionTableIndex lexicon,
	     FlagDiacriticState state):
	string(prev_string),
	input_state(i),
	mutator_state(mutator),
	lexicon_state(lexicon),
	flag_state(state)
	{ }

    TreeNode(FlagDiacriticState start_state): // starting state node
	string(SymbolVector()),
	input_state(0),
	mutator_state(0),
	lexicon_state(0),
	flag_state(start_state)
	{ }

    bool compatible_with(FlagDiacriticOperation op);

    TreeNode update_lexicon(SymbolNumber next_symbol,
			    TransitionTableIndex next_lexicon);

    TreeNode update_mutator(SymbolNumber next_symbol,
			    TransitionTableIndex next_mutator);

    TreeNode update(SymbolNumber next_symbol,
		    unsigned int next_input,
		    TransitionTableIndex next_mutator,
		    TransitionTableIndex next_lexicon);

    TreeNode update(SymbolNumber next_symbol,
		    TransitionTableIndex next_mutator,
		    TransitionTableIndex next_lexicon);
    
};

typedef std::deque<TreeNode> TreeNodeQueue;

int nByte_utf8(unsigned char c);
void debug_print(char * str);

class InputString
{
  
private:
    SymbolVector s;

public:
    InputString():
	s(SymbolVector())
	{ }

    bool initialize(Encoder * encoder, char * input, SymbolNumber other);
    
    const unsigned int len(void)
	{
	    return s.size();
	}

    const SymbolNumber operator[](unsigned int i)
	{
	    return s[i];
	}

};

class Speller
{
public:
    Transducer mutator;
    Transducer lexicon;
    InputString input;
    SymbolVector alphabet_translator;
    std::vector<const char*> * symbol_table;
    
    Speller(FILE * mutator_file, FILE * lexicon_file):
	mutator(Transducer(mutator_file)),
	lexicon(Transducer(lexicon_file)),
	input(InputString()),
	alphabet_translator(SymbolVector()),
	symbol_table(lexicon.get_symbol_table())
	{
	    build_alphabet_translator();
	    mutator.free_temporary();
	    lexicon.free_temporary();
	}
    
    bool init_input(char * str);

    unsigned int get_state_size(void)
	{
	    return mutator.get_state_size();
	}

    void build_alphabet_translator(void);
    void lexicon_epsilons(TreeNodeQueue * queue);
    void mutator_epsilons(TreeNodeQueue * queue);
    void consume_input(TreeNodeQueue * queue);
    bool run(void);
    void output(SymbolVector string);
};

#endif // HFST_OSPELL_OSPELL_H_
