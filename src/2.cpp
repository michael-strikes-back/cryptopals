#include <iostream>
#include <cassert>
#include "encodings.hpp"
#include "main.hpp"
#include "shared.hpp"

static const char in_a[]= "1c0111001f010100061a024b53535009181c";
static const char in_b[]= "686974207468652062756c6c277320657965";

template<>
void problem_node<2>::invoke(const int argc, const char **const argv) {

//Write a function that takes two equal-length buffers and produces their XOR combination.
//
//If your function works properly, then when you feed it the string:
//
//1c0111001f010100061a024b53535009181c
//... after hex decoding, and when XOR'd against:
//
//686974207468652062756c6c277320657965
//... should produce:
//
//746865206b696420646f6e277420706c6179

	uint8_t a[256];
	uint8_t b[256];
	uint8_t out[256];
	size_t an;
	size_t bn;

	if (!hex_decode(in_a, COUNT_OF(in_a), a, COUNT_OF(a), &an)) {
		puts("failed to decode first string");
		return;
	}

	if (!hex_decode(in_b, COUNT_OF(in_b), b, COUNT_OF(b), &bn)) {
		puts("failed to decode second string");
		return;
	}

	xor_repeating(a, an, b, bn, out, COUNT_OF(out));

	for (size_t i= 0; i < an; ++i) {
		printf("%02x", out[i]);
	}
	printf(", %d bytes long\n", static_cast<int32_t>(an));
}

