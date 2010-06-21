#include "ospell.h"

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

bool InputString::initialize(Encoder * encoder, char * input, SymbolNumber other)
{
    // Initialize the symbol vector to the tokenization given by encoder.
    // In the case of tokenization failure, valid utf-8 characters
    // are tokenized as "other" and tokenization is reattempted from
    // such a character onwards. The empty string is tokenized as
    // empty vector; there is no end marker.
    s.clear();
    SymbolNumber k = NO_SYMBOL_NUMBER;
    char ** inpointer = &input;
    char * oldpointer;
    while (**inpointer != '\0') {
	oldpointer = *inpointer;
	k = encoder->find_key(inpointer);
	if (k == NO_SYMBOL_NUMBER) {
	    int n = nByte_utf8(static_cast<unsigned char>(*oldpointer));
	    if (n == 0) {
		return false; // no tokenization
	    } else {
		if (other == NO_SYMBOL_NUMBER) { // no proper other symbol
		    return false; // no tokenization
		}
		oldpointer += n;
		*inpointer = oldpointer;
		s.push_back(other);
		continue;
	    }
	}
	s.push_back(k);
    }
    return true;
}

TreeNode TreeNode::update_lexicon(SymbolNumber next_symbol,
				  TransitionTableIndex next_lexicon)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str, this->input_state, this->mutator_state, next_lexicon, this->flag_state);
}

TreeNode TreeNode::update_mutator(SymbolNumber next_symbol,
				  TransitionTableIndex next_mutator)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str, this->input_state, next_mutator, this->lexicon_state, this->flag_state);
}

TreeNode TreeNode::update(SymbolNumber next_symbol,
			  unsigned int next_input,
			  TransitionTableIndex next_mutator,
			  TransitionTableIndex next_lexicon)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str, next_input, next_mutator, next_lexicon, this->flag_state);
}

TreeNode TreeNode::update(SymbolNumber next_symbol,
			  TransitionTableIndex next_mutator,
			  TransitionTableIndex next_lexicon)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str, this->input_state, next_mutator, next_lexicon, this->flag_state);
}


void Speller::lexicon_epsilons(TreeNodeQueue * queue)
{
    TreeNode front = queue->front();
    int i = 1;
    IndexSymbolPair i_s = lexicon.take_epsilons(front.lexicon_state + i);
    while (i_s.second != NO_SYMBOL_NUMBER) {
	queue->push_back(front.update_lexicon(i_s.second,
					      i_s.first));
	++i;
	i_s = lexicon.take_epsilons(front.lexicon_state + i);
    }
}

void Speller::mutator_epsilons(TreeNodeQueue * queue)
{
    TreeNode front = queue->front();
    int i = 1;
    IndexSymbolPair mutator_i_s = mutator.take_epsilons(front.mutator_state + i);
    while (mutator_i_s.second != NO_SYMBOL_NUMBER) {
	if (mutator_i_s.second == 0) {
	    queue->push_back(front.update_mutator(0,
						  mutator_i_s.first));
	} else {
	    int j = 1;
	    IndexSymbolPair lexicon_i_s = lexicon.take_non_epsilons(front.lexicon_state + j,
								    alphabet_translator[mutator_i_s.second]);
	    while (lexicon_i_s.second != NO_SYMBOL_NUMBER) {
		queue->push_back(front.update(lexicon_i_s.second,
					      mutator_i_s.first,
					      lexicon_i_s.first));
		++j;
		lexicon_i_s = lexicon.take_non_epsilons(front.lexicon_state + j,
							alphabet_translator[mutator_i_s.second]);
	    }
	}
	++i;
	mutator_i_s = mutator.take_epsilons(front.mutator_state + i);
    }
}

void Speller::consume_input(TreeNodeQueue * queue)
{
    TreeNode front = queue->front();
    if (front.input_state + 1 > input.len()) {
	return; // not enough input to consume
    }
    int i = 1;
    IndexSymbolPair mutator_i_s = mutator.take_non_epsilons(front.mutator_state + i,
							    input[front.input_state]);
    while (mutator_i_s.second != NO_SYMBOL_NUMBER) {
	if (mutator_i_s.second == 0) {
	    queue->push_back(front.update(0,
					  front.input_state + 1,
					  mutator_i_s.first,
					  front.lexicon_state));
	} else {
	    int j = 1;
	    IndexSymbolPair lexicon_i_s = lexicon.take_non_epsilons(front.lexicon_state + j,
								    alphabet_translator[mutator_i_s.second]);
	    while (lexicon_i_s.second != NO_SYMBOL_NUMBER) {
		queue->push_back(front.update(lexicon_i_s.second,
					      front.input_state + 1,
					      mutator_i_s.first,
					      lexicon_i_s.first));
		++j;
		lexicon_i_s = lexicon.take_non_epsilons(front.lexicon_state + j,
							alphabet_translator[mutator_i_s.second]);
	    }
	}
     
	++i;
	mutator_i_s = mutator.take_non_epsilons(front.mutator_state + i,
						input[front.input_state]);
    }
}

IndexSymbolPair Transducer::take_epsilons(TransitionTableIndex i)
{
    if (i >= TRANSITION_TARGET_TABLE_START) {
	i -= TRANSITION_TARGET_TABLE_START;
	if (transitions[i]->get_input() != 0) { // not an epsilon transition
	    return IndexSymbolPair(0,NO_SYMBOL_NUMBER);
	}
	return IndexSymbolPair(transitions[i]->target(),
			       transitions[i]->get_output());

    } else {
	if (indices[i] -> get_input() != 0) {
	    return IndexSymbolPair(0, NO_SYMBOL_NUMBER);
	}
	return take_epsilons(indices[i]->target());
    }
}

IndexSymbolPair Transducer::take_non_epsilons(TransitionTableIndex i,
					      SymbolNumber symbol)
{
    if (i >= TRANSITION_TARGET_TABLE_START) {
	i -= TRANSITION_TARGET_TABLE_START;
	if (transitions[i]->get_input() != symbol) {
	    return IndexSymbolPair(0, NO_SYMBOL_NUMBER);
	}
	return IndexSymbolPair(transitions[i]->target(),
			       transitions[i]->get_output());
    } else {
	if (indices[i+symbol]->get_input() != symbol) {
	    return IndexSymbolPair(0, NO_SYMBOL_NUMBER);
	} else {
	    return take_non_epsilons(indices[i+symbol]->target(), symbol);
	}
    }
}

bool Transducer::is_final(TransitionTableIndex i)
{
    if (i >= TRANSITION_TARGET_TABLE_START) {
	return final_transition(i - TRANSITION_TARGET_TABLE_START);
    } else {
	return final_index(i);
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

bool Speller::run(void)
{
    char * str = (char*) malloc(2000);
    TreeNode start_node(FlagDiacriticState(get_state_size(), 0));

    while (!std::cin.eof()) {
	std::cin.getline(str, 2000);
//	std::getline(std::cin, str);
	if (!init_input(str)) {
	    continue; // no tokenization
	}
	TreeNodeQueue state_queue(1, start_node);
	while (state_queue.size() > 0) {
	    lexicon_epsilons(&state_queue);
	    mutator_epsilons(&state_queue);
	    if (state_queue.front().input_state == input.len()) {
		TreeNode front = state_queue.front();
		if (mutator.is_final(front.mutator_state) and
		    lexicon.is_final(front.lexicon_state)) {
		    output(front.string);
		}
	    } else {
		consume_input(&state_queue);
	    }
	    state_queue.pop_front();
	}
    }
    return EXIT_SUCCESS;
}

void Speller::output(SymbolVector string)
{
    for (SymbolVector::iterator it = string.begin(); it != string.end(); ++it) {
	std::cout << symbol_table->operator[](*it);
    }
    std::cout << std::endl;
}

bool Speller::init_input(char * str)
{
    return input.initialize(mutator.get_encoder(), str, mutator.get_other());
}

void Speller::build_alphabet_translator(void)
{
    TransducerAlphabet * from = mutator.get_alphabet();
    TransducerAlphabet * to = lexicon.get_alphabet();
    KeyTable * from_keys = from->get_key_table();
    StringSymbolMap * to_symbols = to->get_string_to_symbol();
    alphabet_translator.push_back(0); // zeroth element is always epsilon
    for (SymbolNumber i = 1; i < from_keys->size(); ++i) {
	if ( (from->is_flag(i)) || // if it's a flag
	     (i == from->get_other()) ) {   // or the OTHER symbol
	    alphabet_translator.push_back(NO_SYMBOL_NUMBER);
	    continue; // no translation
	}
	assert(to_symbols->count(from_keys->operator[](i)) == 1);
	// translator at i points to lexicon's symbol for mutator's string for
	// mutator's symbol number i
	alphabet_translator.push_back(to_symbols->operator[](from_keys->operator[](i)));
    }
}

void debug_print (char * str)
{
#if DEBUG
    std::cerr << str << std::endl;
#endif
}
