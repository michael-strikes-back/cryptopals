/**
 * encodings.hpp
 * 
 * Functions for encoding and decoding data.
 */
#pragma once
#ifndef ENCODINGS_HPP
#define ENCODINGS_HPP

#include "types.hpp"

bool hex_decode(const char *str, size_t strn, byte_t *out_bytes, size_t byten_max, size_t *byten);
size_t hex_encode(const byte_t *bytes, size_t bytesn, char *out_str, size_t strn);

byte_t *base64_decode(const char *data, size_t len, size_t *out_len);
char *base64_encode(const byte_t *data, size_t len, size_t *out_len);

#endif // ENCODINGS_HPP
