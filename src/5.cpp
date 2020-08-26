
#include <cstdio>

#include "shared.hpp"
#include "main.hpp"

static const unsigned char plain[]=
	"Burning 'em, if you ain't quick and nimble\n"
	"I go crazy when I hear a cymbal";

static const byte_t key[]= "ICE";

static unsigned char enciphered[4096];

template<>
void problem_node<5>::invoke(int argc, const char **argv) {
	
	xor_repeating(plain, COUNT_OF(plain)-1, key, COUNT_OF(key)-1, enciphered, COUNT_OF(enciphered)-1);
	enciphered[COUNT_OF(plain)-1]= '\0';
	
	for (size_t i= 0; i < COUNT_OF(plain)-1; ++i) {
		printf("%02x", enciphered[i]);
	}
	printf("\n");
}

