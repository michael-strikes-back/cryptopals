#define FORCE_BYTE_HAMMING_DISTANCE

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "shared.hpp"

#ifdef REMOVED
extern "C" {
	#include "base64.h"
}
#endif // REMOVED

//    private types

struct s_b64_index_table_entry {
	char start;
	byte_t len;
};

//    private data

constexpr char char_freq_order[]= "etaoi nshrdlcumwfgypbvkjxqz";
constexpr char low_freq_characters[]= "-!\":;<=>@#$%&()*+/[\\]^_`{|}~";

constexpr char b64_padding= '=';
constexpr char b64_index_table[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static_assert(sizeof(b64_index_table) == 0x41);

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
	const byte_t *a,
	size_t an,
	const byte_t *b,
	size_t bn,
	byte_t *out,
	size_t outn) {
	size_t ait;
	size_t bit= 0;

	assert(outn >= an);

	for (ait= 0; ait < an; ++ait) {
		out[ait]= a[ait] ^ b[bit];
		bit= (bit+1)%bn;
	}
}

int get_hamming_distance(const byte_t *a, const byte_t *b, size_t count) {
	int num_differing= 0;

#ifndef FORCE_BYTE_HAMMING_DISTANCE
	if (count > 4) {
		for (size_t it= 0; it < count-4; it+= 4) {
			const unsigned av= *static_cast<const unsigned int*>(a+it);
			const unsigned bv= *static_cast<const unsigned int*>(b+it);

			num_differing+= __builtin_popcount(av ^ bv);
		}

		for (size_t it= count - (count % 4); it < count; ++it) {
			byte_t ab= a[it];
			byte_t bb= b[it];

			num_differing+= __builtin_popcount(ab ^ bb);
		}
	} else
#endif // FORCE_BYTE_HAMMING_DISTANCE
	{
		for (size_t it= 0; it < count; ++it) {
			byte_t ab= a[it];
			byte_t bb= b[it];

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
			if (!isprint(c) && c !='\n' && all_freqs[static_cast<byte_t>(c)]==0) {
				non_print_characters++;
			}

			all_freqs[static_cast<byte_t>(c)]++;

			for (freqs_it= 0; freqs_it < freqsn && freqs[freqs_it]!=c; ++freqs_it) { }

			if (freqs_it == freqsn) {
				if (freqs_it < COUNT_OF(freqs)) {
					freqs[freqsn++]= c;
					freqs[freqsn]= '\0';
				}
			} else {
				while (freqs_it > 0 &&
					all_freqs[static_cast<byte_t>(freqs[freqs_it])] > all_freqs[static_cast<byte_t>(freqs[freqs_it-1])]) {

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

bool hex_decode(const char *str, size_t strn, byte_t *out_bytes, size_t byten_max, size_t *out_byten) {
	const char num_start= '0';
	const char num_end= '9';
	const char alpha_start= 'a';
	const char alpha_end= 'f';
	const char lower_case_diff= 'a' - 'A';

	bool success= true;
	size_t i;
	for (i= 0; success && str[i]!='\0'; i+= 2) {
		if (i+1 > strn) { success= false; break; }
		if (i/2 > byten_max) { success= false; break; }

		char hi_nibble= str[i];
		char lo_nibble= str[i+1];

		hi_nibble+= in_range(hi_nibble, 'A', 'Z') ? lower_case_diff : 0;
		lo_nibble+= in_range(lo_nibble, 'A', 'Z') ? lower_case_diff : 0;

		if (in_range(hi_nibble, num_start, num_end)) {
			hi_nibble= hi_nibble - num_start;
		} else if (in_range(hi_nibble, alpha_start, alpha_end)) {
			hi_nibble= hi_nibble - alpha_start + 0xa;
		} else {
			success= false;
		}

		if (in_range(lo_nibble, num_start, num_end)) {
			lo_nibble= lo_nibble - num_start;
		} else if (in_range(lo_nibble, alpha_start, alpha_end)) {
			lo_nibble= lo_nibble - alpha_start + 0xa;
		} else {
			success= false;
		}

		out_bytes[i/2]= (hi_nibble << 4) | lo_nibble;
	}

	*out_byten= i/2;

	return success;
}

void hex_encode(const unsigned char *bytes, size_t bytesn, char *out_str, size_t strn) {
	assert(strn >= 1);
	// $TODO stub
	out_str[0]= '\0';
}

byte_t *base64_decode(const char *const data, size_t len, size_t *out_len) {
	assert(data);
	assert(out_len);

	*out_len= 0;

	return nullptr;
}

char *base64_encode(const byte_t *const data, size_t len, size_t *out_len) {
	assert(data);
	assert(out_len);

	// calculate the base64-encoded length
	{
		const size_t out_len_truncated= 4 * len / 3;
		// +1 for null-terminator
		// pad to the nearest multiple of 4
		*out_len= 1 +
			(((out_len_truncated & 3) != 0) ? ((out_len_truncated & ~3) + 4) : out_len_truncated);
	}

	char *const out_buffer= new char[*out_len];

	const size_t len_truncated= len / 3 * 3;
	size_t offset_group= 0;
	size_t output_index= 0;

	constexpr unsigned int sextet= 0x3f;
	constexpr unsigned int bits_per_character_group= 24;
	constexpr unsigned int octet_size= 8;
	constexpr unsigned int sextet_size= 6;
	constexpr unsigned int octets_per_character_group= bits_per_character_group/octet_size;
	constexpr unsigned int octet_count= bits_per_character_group/octet_size;
	constexpr unsigned int sextet_count= bits_per_character_group/sextet_size;

	for (; offset_group < len_truncated; offset_group+= octets_per_character_group) {
		register unsigned int joined= 0;
		for (unsigned int octet_index= 0; octet_index < octet_count; ++octet_index) {
			// in a given group, the hi sextet is the first char, and lo sextet is the last char.
			const size_t data_index= offset_group + octet_count - 1 - octet_index;
			joined|= data[data_index] << (octet_index*octet_size);
		}
		for (int sextet_index= sextet_count-1; sextet_index >= 0; --sextet_index) {
			const int sextet_offset= sextet_index * sextet_size;
			out_buffer[output_index++]= b64_index_table[(joined & (sextet << sextet_offset)) >> sextet_offset];
		}
	}

	// calculate remainder with padding
	if (len != len_truncated) {
		register unsigned int joined= 0;
		int lo_used_octet_offset= 0;

		for (int octet_index= octet_count - 1; octet_index >= 0; --octet_index) {
			const size_t data_index= offset_group + octet_index;
			if (data_index < len) {
				const int octet_offset= (octet_count - 1 - octet_index)*octet_size;
				joined|= data[data_index] << octet_offset;
				lo_used_octet_offset= octet_offset;
			}
		}

		// where a&b are remainder:
		//    321098765432109876543210
		//    aaaaaaaabbbbbbbbcccccccc
		// lo_used_octet_offset will be 8
		// result base64:
		//    ddddddeeeeeeffffff======

		for (int sextet_index= sextet_count-1; sextet_index >= 0; --sextet_index) {
			const int sextet_offset= sextet_index * sextet_size;
			if (sextet_offset > lo_used_octet_offset) {
				out_buffer[output_index++]= b64_index_table[(joined & (sextet << sextet_offset)) >> sextet_offset];
			} else {
				out_buffer[output_index++]= b64_padding;
			}
		}
	}

	out_buffer[*out_len-1]= '\0';
	return out_buffer;
}

byte_t *get_enciphered_text_from_base64_file(
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

	byte_t *enciphered= base64_decode(base64_encoded, file_size, out_enciphered_len);
	assert(enciphered);

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
