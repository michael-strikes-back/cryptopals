
#pragma once
#ifndef SHARED_HPP
#define SHARED_HPP

#include <cstddef>
#include <cstdint>

// constants

// definitions

#define COUNT_OF(a) (sizeof(a)/sizeof((a)[0]))

typedef unsigned char byte_t;

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

bool hex_decode(const char *str, size_t strn, byte_t *out_bytes, size_t byten_max, size_t *byten);
size_t hex_encode(const byte_t *bytes, size_t bytesn, char *out_str, size_t strn);

byte_t *base64_decode(const char *data, size_t len, size_t *out_len);
char *base64_encode(const byte_t *data, size_t len, size_t *out_len);

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

