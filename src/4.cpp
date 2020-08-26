
#include <cassert>
#include <cstdio>
#include <cstring>

#include "shared.hpp"
#include "main.hpp"

static const size_t k_linebuf_count= 4096*2;
static const size_t k_textbuf_count= 4096;

static char line[k_linebuf_count];
static char enciphered[k_textbuf_count];
static char plain[k_textbuf_count];

template<>
void problem_node<4>::invoke(int argc, const char **argv) {
	int best_score= -1;

	if (argc != 1) {
		fputs("Usage: -4 [data file]\n", stderr);
		return;
	}

	FILE *const f= fopen(argv[0], "r");
	if (f == nullptr) {
		fputs("failed to open file for reading.\n", stderr);
		return;
	}

	while (nullptr != fgets(line, COUNT_OF(line), f)) {
		size_t enciphered_len;

		// trim newlines
		line[strcspn(line, "\r\n")]= '\0';

		{
			byte_t *const enciphered_ptr= reinterpret_cast<unsigned char *>(enciphered);
			if (!hex_decode(line, COUNT_OF(line), enciphered_ptr, COUNT_OF(enciphered), &enciphered_len)) {
				printf("failed to decode string '%s'\n", line);
				break;
			}
		}

		for (unsigned short key= 0; key < 256; ++key) {
			size_t enciphered_index= 0;
			for (; enciphered_index < enciphered_len; ++enciphered_index) {
				plain[enciphered_index]= enciphered[enciphered_index] ^ key;
			}
			plain[enciphered_index]= '\0';

			int score= score_plain_text(plain, enciphered_len);

			if (score > best_score) {
				best_score= score;
			}
		}
	}

	fclose(f);

	printf("best score: %d\n", best_score);
}

