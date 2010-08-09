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

bool InputString::initialize(Encoder * encoder,
			     char * input,
			     SymbolNumber other)
{
    // Initialize the symbol vector to the tokenization given by encoder.
    // In the case of tokenization failure, valid utf-8 characters
    // are tokenized as "other" and tokenization is reattempted from
    // such a character onwards. The empty string is tokenized as
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

TreeNode TreeNode::update_lexicon(SymbolNumber next_symbol,
				  TransitionTableIndex next_lexicon,
				  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str,
		    this->input_state,
		    this->mutator_state,
		    next_lexicon,
		    this->flag_state,
		    this->weight + weight);
}

TreeNode TreeNode::update_mutator(SymbolNumber next_symbol,
				  TransitionTableIndex next_mutator,
				  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str,
		    this->input_state,
		    next_mutator,
		    this->lexicon_state,
		    this->flag_state,
		    this->weight + weight);
}

TreeNode TreeNode::update(SymbolNumber next_symbol,
			  unsigned int next_input,
			  TransitionTableIndex next_mutator,
			  TransitionTableIndex next_lexicon,
			  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
    return TreeNode(str,
		    next_input,
		    next_mutator,
		    next_lexicon,
		    this->flag_state,
		    this->weight + weight);
}

TreeNode TreeNode::update(SymbolNumber next_symbol,
			  TransitionTableIndex next_mutator,
			  TransitionTableIndex next_lexicon,
			  Weight weight)
{
    SymbolVector str(this->string);
    str.push_back(next_symbol);
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

void Speller::lexicon_epsilons(void)
{
    TreeNode front = queue.front();
    if (!lexicon->has_epsilons_or_flags(front.lexicon_state + 1)) {
	return;
    }
    TransitionTableIndex next = lexicon->next(front.lexicon_state, 0);
    // FIXME: if the epsilon transition area begins with a flag, this breaks
    STransition i_s = lexicon->take_epsilons_and_flags(next);
    
    while (i_s.symbol != NO_SYMBOL) {
	if ((lexicon->transitions[next]->get_input() == 0) or
	    front.try_compatible_with( // this is terrible
		operations->operator[](
		    lexicon->transitions[next]->get_input()))) {
	    queue.push_back(front.update_lexicon(i_s.symbol,
						 i_s.index,
						 i_s.weight));
	}
	++next;
	i_s = lexicon->take_epsilons_and_flags(next);
    }
}

void Speller::lexicon_consume(void)
{
    TreeNode front = queue.front();
    unsigned int input_state = front.input_state;
    if (input_state + 1 > input.len() or
	!lexicon->has_transitions(
	    front.lexicon_state + 1, input[input_state])) {
	return;
    }
    
    TransitionTableIndex next = lexicon->next(front.lexicon_state,
					      input[input_state]);
    STransition i_s = lexicon->take_non_epsilons(next,
						 input[input_state]);

    while (i_s.symbol != NO_SYMBOL) {
	queue.push_back(front.update(
			    i_s.symbol,
			    input_state + 1,
			    front.mutator_state,
			    i_s.index,
			    i_s.weight));
	
	++next;
	i_s = lexicon->take_non_epsilons(next, input[input_state]);
    }
    
}

void Speller::mutator_epsilons(void)
{
    TreeNode front = queue.front();
    if (!mutator->has_transitions(front.mutator_state + 1, 0)) {
	return;
    }
    TransitionTableIndex next_m = mutator->next(front.mutator_state, 0);
    STransition mutator_i_s = mutator->take_epsilons(next_m);
   
    while (mutator_i_s.symbol != NO_SYMBOL) {
	if (mutator_i_s.symbol == 0) {
	    queue.push_back(front.update_mutator(mutator_i_s.symbol,
						 mutator_i_s.index,
						 mutator_i_s.weight));
	} else {
	    if (!lexicon->has_transitions(
		    front.lexicon_state + 1,
		    alphabet_translator[mutator_i_s.symbol])) {
		++next_m;
		mutator_i_s = mutator->take_epsilons(next_m);
		continue;
	    }
	    TransitionTableIndex next_l = lexicon->next(
		front.lexicon_state,
		alphabet_translator[mutator_i_s.symbol]);
	    STransition lexicon_i_s = lexicon->take_non_epsilons(
		next_l,
		alphabet_translator[mutator_i_s.symbol]);
	    
	    while (lexicon_i_s.symbol != NO_SYMBOL) {
		queue.push_back(front.update(lexicon_i_s.symbol,
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
    unsigned int input_state = queue.front().input_state;
    if (input_state + 1 > input.len()) {
	return; // not enough input to consume
    }
    
    TreeNode front = queue.front();
    if (!mutator->has_transitions(front.mutator_state + 1,
				  input[input_state])) {
	return;
    }
    
    TransitionTableIndex next_m = mutator->next(front.mutator_state,
						input[input_state]);
    
    STransition mutator_i_s = mutator->take_non_epsilons(next_m,
							 input[input_state]);
    
    while (mutator_i_s.symbol != NO_SYMBOL) {

	if (mutator_i_s.symbol == 0) {
	    
	    queue.push_back(front.update(0,
					 input_state + 1,
					 mutator_i_s.index,
					 front.lexicon_state,
					 mutator_i_s.weight));
	} else {
	    if (!lexicon->has_transitions(
		    front.lexicon_state + 1,
		    alphabet_translator[mutator_i_s.symbol])) {
		++next_m;
		mutator_i_s = mutator->take_non_epsilons(next_m,
							 input[input_state]);
		continue;
	    }
	    TransitionTableIndex next_l = lexicon->next(
		front.lexicon_state,
		alphabet_translator[mutator_i_s.symbol]);
	    
	    STransition lexicon_i_s = lexicon->take_non_epsilons(
		next_l,
		alphabet_translator[mutator_i_s.symbol]);
	    
	    while (lexicon_i_s.symbol != NO_SYMBOL) {
		queue.push_back(
		    front.update(lexicon_i_s.symbol,
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

TransitionTableIndex Transducer::next(TransitionTableIndex i,
				      SymbolNumber symbol)
{
    if (i >= TARGET_TABLE) {
	return i - TARGET_TABLE + 1;
    } else {
	return indices[i+1+symbol]->target() - TARGET_TABLE;
    }
}

bool Transducer::has_transitions(TransitionTableIndex i,
				 SymbolNumber symbol)
{
    if (i >= TARGET_TABLE) {
	return (transitions[i - TARGET_TABLE]->get_input() == symbol);
    } else {
	return (indices[i+symbol]->get_input() == symbol);
    }
}

bool Transducer::has_epsilons_or_flags(TransitionTableIndex i)
{
    if (i >= TARGET_TABLE) {
	return(transitions[i - TARGET_TABLE]->get_input() == 0 or
	       is_flag(transitions[i - TARGET_TABLE]->get_input()));
    } else {
	return (indices[i]->get_input() == 0);
    }
}

STransition Transducer::take_epsilons(TransitionTableIndex i)
{
    if (transitions[i]->get_input() != 0) {
	return STransition(0, NO_SYMBOL);
    }
    return STransition(transitions[i]->target(),
		       transitions[i]->get_output(),
		       transitions[i]->get_weight());
}

STransition Transducer::take_epsilons_and_flags(TransitionTableIndex i)
{
    if (transitions[i]->get_input() != 0 and
	!is_flag(transitions[i]->get_input())) {
	return STransition(0,NO_SYMBOL);
    }
    return STransition(transitions[i]->target(),
		       transitions[i]->get_output(),
		       transitions[i]->get_weight());
}

STransition Transducer::take_non_epsilons(TransitionTableIndex i,
					  SymbolNumber symbol)
{
    if (transitions[i]->get_input() != symbol) {
	return STransition(0, NO_SYMBOL);
    }
    return STransition(transitions[i]->target(),
		       transitions[i]->get_output(),
		       transitions[i]->get_weight());
}

bool Transducer::is_final(TransitionTableIndex i)
{
    if (i >= TARGET_TABLE) {
	return final_transition(i - TARGET_TABLE);
    } else {
	return final_index(i);
    }
}

Weight Transducer::final_weight(TransitionTableIndex i)
{
    if (i >= TARGET_TABLE) {
	return transitions[i - TARGET_TABLE]->get_weight();
    } else {
	return indices[i]->final_weight();
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

CorrectionQueue Speller::correct(char * line)
{
    if (!init_input(line, mutator->get_encoder(), mutator->get_other())) {
	return CorrectionQueue();
    }
    std::map<std::string, Weight> corrections;
    TreeNode start_node(FlagDiacriticState(get_state_size(), 0));
    queue.assign(1, start_node);

    while (queue.size() > 0) {
	lexicon_epsilons();
	mutator_epsilons();
	if (queue.front().input_state == input.len()) {
	    TreeNode front = queue.front();
	    std::string string = stringify(front.string);
	    Weight weight = front.weight +
		lexicon->final_weight(front.lexicon_state) +
		mutator->final_weight(front.mutator_state);

	    /*
	     * The following condition means: if all our transducers are in
	     * final states AND the correction is either new or has a lower
	     * weight than before, we make it a correction
	     */
	    if (mutator->is_final(front.mutator_state) and
		lexicon->is_final(front.lexicon_state) and
		((corrections.count(string)) == 0 or
		 corrections[string] > weight)) {
		corrections[string] = weight;
	    }
	} else {
	    consume_input();
	}
	queue.pop_front();
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
	if (queue.front().input_state == input.len() and
	    lexicon->is_final(queue.front().lexicon_state)) {
	    return true;
	}
	lexicon_epsilons();
	lexicon_consume();
	queue.pop_front();
    }
    return false;
}

std::string Speller::stringify(SymbolVector symbol_vector)
{
    std::string s;
    for (SymbolVector::iterator it = symbol_vector.begin();
	 it != symbol_vector.end(); ++it) {
	s.append(symbol_table->operator[](*it));
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
	    throw AlphabetTranslationException();
	}
	// translator at i points to lexicon's symbol for mutator's string for
	// mutator's symbol number i
	alphabet_translator.push_back(
	    to_symbols->operator[](
		from_keys->operator[](i)));
    }
}

} // namespace hfst_ol
    
