
#include <cassert>
#include <cstdio>
#include <cstring>

#include "encodings.hpp"
#include "main.hpp"
#include "shared.hpp"

constexpr const char *cases[]= {
	"49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d",
		"SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t",
	"616e79206361726e616c20706c656173757265", "YW55IGNhcm5hbCBwbGVhc3VyZQ==",
	"616e79206361726e616c20706c6561737572", "YW55IGNhcm5hbCBwbGVhc3Vy",
	"616e79206361726e616c20706c65617375", "YW55IGNhcm5hbCBwbGVhc3U=",
};

template<>
void problem_node<1>::invoke(const int argc, const char **const argv) {

	size_t case_index;
	byte_t a[256];
	size_t an;
	size_t base64_len;

	// run a few test cases on hex and base64 encode/decode
	for (case_index= 0; case_index < COUNT_OF(cases); case_index+= 2) {

		if (!hex_decode(cases[case_index], strlen(cases[case_index])+1, a, COUNT_OF(a), &an)) {
			puts("failed to decode string");
			return;
		}

		// extra verification check for hex_decode - encode it again to ensure equality.
		{
			char case_reencoded[256];
			size_t case_reencoded_len= hex_encode(a, an, case_reencoded, 256);
			assert(case_reencoded_len==strlen(cases[case_index]));
			for (size_t reencoded_index= 0; reencoded_index < case_reencoded_len; ++reencoded_index) {
				if (case_reencoded[reencoded_index] != cases[case_index][reencoded_index]) {
					fprintf(stderr, "'%c' != '%c'\n", case_reencoded[reencoded_index], cases[case_index][reencoded_index]);
					break;
				}
			}
		}

		char *const base64= base64_encode(a, an, &base64_len);

		if (0 != strcmp(base64, cases[case_index+1])) {
			printf("Got '%s'\n", base64);
			printf("Expected '%s'\n", cases[case_index+1]);
		}

		delete[](base64);
	}

	puts("done");
}
