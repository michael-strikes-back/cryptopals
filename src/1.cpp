
#include <cassert>
#include <cstdio>
#include <cstring>

#include "encodings.hpp"
#include "main.hpp"
#include "shared.hpp"

constexpr const char *cases[]= {
	"49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d",
		"SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t",
	// full range of padding cases
	"616e79206361726e616c20706c656173757265", "YW55IGNhcm5hbCBwbGVhc3VyZQ==",
	"616e79206361726e616c20706c6561737572",   "YW55IGNhcm5hbCBwbGVhc3Vy",
	"616e79206361726e616c20706c65617375",     "YW55IGNhcm5hbCBwbGVhc3U=",
	"616e79206361726e616c20706c656173",       "YW55IGNhcm5hbCBwbGVhcw==",
	"616e79206361726e616c20706c6561",         "YW55IGNhcm5hbCBwbGVh",
	// edge cases
	"", ""
};

template<>
void problem_node<1>::invoke(const int argc, const char **const argv) {

	size_t case_index;
	uint8_t a[256];
	size_t an;

	// run a few test cases on hex and base64 encode/decode
	for (case_index= 0; case_index < COUNT_OF(cases); case_index+= 2) {

		if (!hex_decode(cases[case_index], strlen(cases[case_index])+1, a, COUNT_OF(a), &an)) {
			puts("failed to decode string");
			return;
		}

		// extra verification check for hex_decode - encode it again to ensure equality.
		{
			char case_reencoded[256];
			const size_t case_reencoded_len= hex_encode(a, an, case_reencoded, 256);
			assert(case_reencoded_len==strlen(cases[case_index]));

			for (size_t reencoded_index= 0; reencoded_index < case_reencoded_len; ++reencoded_index) {
				if (case_reencoded[reencoded_index] != cases[case_index][reencoded_index]) {
					fprintf(stderr, "'%c' != '%c'\n", case_reencoded[reencoded_index], cases[case_index][reencoded_index]);
					break;
				}
			}
		}

		size_t b64encoded_len;
		char *const b64encoded= base64_encode(a, an, &b64encoded_len);

		if (0 != strcmp(b64encoded, cases[case_index+1])) {
			printf("Got '%s'\n", b64encoded);
			printf("Expected '%s'\n", cases[case_index+1]);
		}

		// decode again to compare
		{
			size_t b64decoded_len;
			uint8_t *b64decoded;
			
			if (!base64_decode(b64encoded, b64encoded_len, &b64decoded, &b64decoded_len)) {
				printf("invalid base64?!\n");
				b64decoded= nullptr;
			} else if (b64decoded_len != an) {
				printf("Length mismatch, got '%zd', expected '%zd'\n", b64decoded_len, an);
			} else {
				char reencoded_hex[1024];
				hex_encode(b64decoded, b64decoded_len, reencoded_hex, COUNT_OF(reencoded_hex));
				if (strcmp(reencoded_hex, cases[case_index]) != 0) {
					printf("Got '%s', expected '%s'\n", reencoded_hex, cases[case_index]);
				}
			}
			
			if (b64decoded) {
				delete[](b64decoded);
			}
		}

		delete[](b64encoded);
	}

	puts("done");
}
