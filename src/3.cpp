#include <iostream>
#include <cassert>
#include <cstring>
#include "encodings.hpp"
#include "main.hpp"
#include "shared.hpp"

static const char in_hex[]= "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";

template<>
void problem_node<3>::invoke(const int argc, const char **const argv) {
//The hex encoded string:
//
//1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736
//... has been XOR'd against a single character. Find the key, decrypt the message.
//
//You can do this by hand. But don't: write code to do it for you.
//
//How? Devise some method for "scoring" a piece of English plaintext. Character frequency is a good metric. Evaluate each output and choose the one with the best score.

	byte_t a[256];
	char b[256];
	size_t an;
	size_t bn;

	if (!hex_decode(in_hex, COUNT_OF(in_hex), a, COUNT_OF(a), &an)) {
		puts("failed to decode string");
		return;
	}

	int top_score= -1;
	unsigned char top_key= 0;
	unsigned short key;

	bn= an;

	for (key= 0; key < 256; ++key) {

		for (size_t ait= 0; ait < an; ++ait) {
			b[ait]= a[ait] ^ key;
		}

		int score= score_plain_text(b, an);

		if (score > 17) {
			printf("score for '%s': %d\n", static_cast<const char *>(b), score);
		}

		if (score > top_score) {
			top_score= score;
			top_key= key;
		}
	}

	for (size_t ait= 0; ait < an; ++ait) {
		b[ait]= a[ait] ^ top_key;
	}

	b[bn]= '\0';
	printf("%s\n", b);
	for (size_t i= 0; i < an; ++i) {
		printf("%02x", b[i]);
	}
	printf(", %d bytes long\n", static_cast<int32_t>(an));
}

