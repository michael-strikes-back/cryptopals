#define FORCE_BYTE_HAMMING_DISTANCE

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "shared.hpp"
#include "encodings.hpp"

//    private types

//    private data

constexpr char char_freq_order[]= "etaoi nshrdlcumwfgypbvkjxqz";
constexpr char low_freq_characters[]= "-!\":;<=>@#$%&()*+/[\\]^_`{|}~";

//    private prototypes

static int score_top_chars(const char *top_chars, size_t len);

//    public definitions

template<typename t_comparable>
inline bool in_range(
	const t_comparable in,
	const t_comparable lower_inclusive,
	const t_comparable upper_inclusive) {
	return in >= lower_inclusive && in <= upper_inclusive;
}

void xor_repeating(
	const uint8_t *a,
	size_t an,
	const uint8_t *b,
	size_t bn,
	uint8_t *out,
	size_t outn) {
	size_t ait;
	size_t bit= 0;

	assert(outn >= an);

	for (ait= 0; ait < an; ++ait) {
		out[ait]= a[ait] ^ b[bit];
		bit= (bit+1)%bn;
	}
}

int get_hamming_distance(const uint8_t *a, const uint8_t *b, size_t count) {
	int num_differing= 0;

#ifndef FORCE_BYTE_HAMMING_DISTANCE
	if (count > 4) {
		for (size_t it= 0; it < count-4; it+= 4) {
			const unsigned av= *static_cast<const unsigned int*>(a+it);
			const unsigned bv= *static_cast<const unsigned int*>(b+it);

			num_differing+= __builtin_popcount(av ^ bv);
		}

		for (size_t it= count - (count % 4); it < count; ++it) {
			uint8_t ab= a[it];
			uint8_t bb= b[it];

			num_differing+= __builtin_popcount(ab ^ bb);
		}
	} else
#endif // FORCE_BYTE_HAMMING_DISTANCE
	{
		for (size_t it= 0; it < count; ++it) {
			uint8_t ab= a[it];
			uint8_t bb= b[it];

			num_differing+= __builtin_popcount(ab ^ bb);
		}
	}

	return num_differing;
}

class c_contiguous_iterator : public c_string_iterator_interface {
public:
	c_contiguous_iterator(const char *chars, size_t count)
		: m_it(0)
		, m_chars(chars)
		, m_count(count)
		{ }

	virtual bool has_next() { return m_it < m_count && m_chars[m_it]!='\0'; }
	virtual char current() { return m_chars[m_it]; }
	virtual void next() { ++m_it; }

private:
	size_t m_it;
	const char *m_chars;
	size_t m_count;
};

c_string_iterator_interface *current_it;

int score_plain_text(const char *const plain, const size_t plain_count) {
	c_contiguous_iterator iterator(plain, plain_count);

	current_it= &iterator;

	return score_plain_text(iterator);
}

int score_plain_text(c_string_iterator_interface &it) {
	int non_print_characters= 0;
	char freqs[COUNT_OF(char_freq_order)];
	freqs[0]= '\0';
	
	{
		size_t freqsn= 0;
		size_t freqs_it;
		// lazy
		size_t all_freqs[256];
		memset(all_freqs, 0, sizeof(all_freqs));

		for (; it.has_next(); it.next()) {
			const char c= tolower(it.current());

			// special: count characters that are not printable
			if (!isprint(c) && c !='\n' && all_freqs[static_cast<uint8_t>(c)]==0) {
				non_print_characters++;
			}

			all_freqs[static_cast<uint8_t>(c)]++;

			for (freqs_it= 0; freqs_it < freqsn && freqs[freqs_it]!=c; ++freqs_it) { }

			if (freqs_it == freqsn) {
				if (freqs_it < COUNT_OF(freqs)) {
					freqs[freqsn++]= c;
					freqs[freqsn]= '\0';
				}
			} else {
				while (freqs_it > 0 &&
					all_freqs[static_cast<uint8_t>(freqs[freqs_it])] > all_freqs[static_cast<uint8_t>(freqs[freqs_it-1])]) {

					const char t= freqs[freqs_it];
					freqs[freqs_it]= freqs[freqs_it-1];
					freqs[freqs_it-1]= t;

					freqs_it--;
				}
			}
		}
	}

	int score= score_top_chars(freqs, strlen(freqs));
	score= max(0, score - non_print_characters*2);

	// special: look at the upper half frequency chars for unexpected values
	int unexpected_lo_frequency_chars= 0;
	{
		const char temp_mid_char= freqs[COUNT_OF(freqs)/2];
		freqs[COUNT_OF(freqs)/2]= '\0';

		const char *lo_freq_ptr= strpbrk(freqs, low_freq_characters);
		for (; lo_freq_ptr != nullptr; unexpected_lo_frequency_chars++) {
			// increment to skip the found pointer
			lo_freq_ptr= strpbrk(lo_freq_ptr+1, low_freq_characters);
		}

		freqs[COUNT_OF(freqs)/2]= temp_mid_char;
	}

	score= max(0, score - unexpected_lo_frequency_chars);

	return score;
}

uint8_t *get_enciphered_text_from_base64_file(
	const char *const file_name,
	size_t *const out_enciphered_len) {

	FILE *const f= fopen(file_name, "rb");

	if (nullptr == f) {
		fputs("file could not be opened\n", stderr);
		return nullptr;
	}

	if (fseek(f, 0, SEEK_END)) {
		fputs("failed to get file size\n", stderr);
		return nullptr;
	}
	const size_t file_size= ftell(f);

	fseek(f, 0, SEEK_SET);

	char *const base64_encoded= new char[file_size];
	assert(base64_encoded);

	if (file_size != fread(base64_encoded, 1, file_size, f)) {
		fputs("failed to read contents\n", stderr);
		return nullptr;
	}
	fclose(f);

	uint8_t *enciphered;
	bool decoded= base64_decode(base64_encoded, file_size, &enciphered, out_enciphered_len);
	assert(decoded);

	delete[](base64_encoded);

	return enciphered;
}

//    private definitions

static int score_top_chars(
	const char *top_chars,
	size_t len) {
	int score= 0;
	size_t it, it2;
	const size_t k_max_extent_to_check= 6;

	const size_t extent_to_check= min(len, k_max_extent_to_check);

	// hi frequency
	for (it= 0; it < extent_to_check; ++it) {
		char test_char= tolower(top_chars[it]);

		for (it2= 0; it2 < extent_to_check; ++it2) {
			score += test_char==char_freq_order[it2] ? 2 : 0;
		}
	}

	// lo frequency
	for (it= len-1; it > len-extent_to_check-1; --it) {
		char test_char= tolower(top_chars[it]);

		for (it2= len-1; it2 > len-extent_to_check-1; --it2) {
			score += test_char==char_freq_order[it2] ? 2 : 0;
		}
	}

	return score;
}
