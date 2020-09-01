
#pragma once
#ifndef SHARED_HPP
#define SHARED_HPP

#include "macros.hpp"
#include "types.hpp"

// constants

// definitions

class c_string_iterator_interface {
public:
	virtual bool has_next()= 0;
	virtual char current()= 0;
	virtual void next()= 0;
};

// functions

template<typename t> t max(t a, t b);
template<typename t> t min(t a, t b);

int get_hamming_distance(const byte_t *a, const byte_t *b, size_t count);

int score_plain_text(const char *a, const size_t an);
int score_plain_text(c_string_iterator_interface &it);

void xor_repeating( const byte_t *a, size_t an, const byte_t *b, size_t bn, byte_t *out, size_t outn);

byte_t *get_enciphered_text_from_base64_file(
	const char *file_name,
	size_t *out_enciphered_len);

template<typename t>
t max(t a, t b) {
	return a > b ? a : b;
}

template<typename t>
t min(t a, t b) {
	return a < b ? a : b;
}

#endif // SHARED_HPP

