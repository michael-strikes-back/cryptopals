
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "encodings.hpp"
#include "macros.hpp"

//    private data

constexpr char b64_padding= '=';
constexpr char b64_index_table[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static_assert(sizeof(b64_index_table) == 0x41);

constexpr unsigned int b64_sextet= 0x3f;
constexpr unsigned int b64_bits_per_character_group= 24;
constexpr unsigned int b64_octet_size= 8;
constexpr unsigned int b64_sextet_size= 6;
constexpr unsigned int b64_octet_count_per_group= b64_bits_per_character_group/b64_octet_size;
constexpr unsigned int b64_sextet_count_per_group= b64_bits_per_character_group/b64_sextet_size;

//    public definitions

bool hex_decode(const char *str, size_t strn, byte_t *out_bytes, size_t byten_max, size_t *out_byten) {
	static constexpr union {
		// zero when not a valid nibble character
		// otherwise, negate to get the mapped value
		char negated_nibble_if_valid;
	} decode_table[]= {
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x10
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x20
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x30
		{~0x0},{~0x1},{~0x2},{~0x3},{~0x4},{~0x5},{~0x6},{~0x7},{~0x8},{~0x9},{},{},{},{},{},{}, // 0x40
		{},{~0xA},{~0xB},{~0xC},{~0xD},{~0xE},{~0xF},{},{},{},{},{},{},{},{},{},                 // 0x50
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x60
		{},{~0xa},{~0xb},{~0xc},{~0xd},{~0xe},{~0xf},{},{},{},{},{},{},{},{},{},                 // 0x70
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x80
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x90
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xa0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xb0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xc0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xd0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xe0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0xf0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x100
	};
	static_assert(COUNT_OF(decode_table) == 0x100);

	bool success= true;
	size_t i;
	// $TODO if better perf needed, can be converted to unrolled, see example technique in hex_encode
	for (i= 0; success && str[i]!='\0'; i+= 2) {
		if (i+1 > strn) {
			success= false;
			break;
		}
		if (i/2 > byten_max) {
			success= false;
			break;
		}

		char hi_nibble= decode_table[static_cast<unsigned char>(str[i])].negated_nibble_if_valid;
		char lo_nibble= decode_table[static_cast<unsigned char>(str[i+1])].negated_nibble_if_valid;

		// check for success
		success= hi_nibble && lo_nibble;
		hi_nibble= ~hi_nibble;
		lo_nibble= ~lo_nibble;

		out_bytes[i/2]= (hi_nibble << 4) | lo_nibble;
	}

	*out_byten= i/2;

	return success;
}

size_t hex_encode(const byte_t *bytes, size_t bytesn, char *out_str, size_t str_max) {

	// I was lazy and made assumptions when I wrote this.
	static_assert(sizeof(char)==1);

	assert(str_max >= bytesn * 2 + 1);
	constexpr char lookup[]= "0123456789abcdef";

	size_t i= 0;

	// explicitly unroll for the bulk of the data
	const size_t bytesn_aligned= (bytesn >> 4) << 4;
	for (; i < bytesn_aligned; i+= 0x10) {
		for (int offset= 0; offset < 0x10; ++offset) {
			out_str[(i+offset)*2]= lookup[bytes[(i+offset)]>>4];
			out_str[(i+offset)*2 + 1]= lookup[bytes[(i+offset)]&0xf];
		}
	}

	// remainder
	for (; i < bytesn; ++i) {
		out_str[i*2]= lookup[bytes[i]>>4];
		out_str[i*2 + 1]= lookup[bytes[i]&0xf];
	}

	out_str[i*2]= '\0';

	// return the length of the encoded string
	return i*2;
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

	const size_t len_common_factor= len / b64_octet_count_per_group;
	const size_t len_truncated= len_common_factor * b64_octet_count_per_group;

	// calculate the base64-encoded length
	{
		const size_t out_len_truncated= len_common_factor * b64_sextet_count_per_group;
		// pad to the nearest multiple
		*out_len= ((len_truncated != len) ? (out_len_truncated + b64_sextet_count_per_group) : out_len_truncated);
	}

	// + 1 for null terminator
	char *const out_buffer= new char[*out_len + 1];

	size_t offset_group= 0;
	size_t output_index= 0;

	for (; offset_group < len_truncated; offset_group+= b64_octet_count_per_group) {
		register unsigned int joined= 0;
		for (unsigned int octet_index= 0; octet_index < b64_octet_count_per_group; ++octet_index) {
			// in a given group, the hi b64_sextet is the first char, and lo b64_sextet is the last char.
			const size_t data_index= offset_group + b64_octet_count_per_group - 1 - octet_index;
			joined|= data[data_index] << (octet_index*b64_octet_size);
		}
		for (int sextet_index= b64_sextet_count_per_group-1; sextet_index >= 0; --sextet_index) {
			const int sextet_offset= sextet_index * b64_sextet_size;
			out_buffer[output_index++]= b64_index_table[(joined & (b64_sextet << sextet_offset)) >> sextet_offset];
		}
	}

	// calculate remainder with padding
	if (len != len_truncated) {
		register unsigned int joined= 0;
		int lo_used_octet_offset= 0;

		for (unsigned int octet_index= 0; octet_index < b64_octet_count_per_group; ++octet_index) {
			// in a given group, the hi b64_sextet is the first char, and lo b64_sextet is the last char.
			const size_t data_index= offset_group + octet_index;
			if (data_index < len) {
				const int octet_offset= (b64_octet_count_per_group - octet_index - 1) * b64_octet_size;
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

		for (int sextet_index= b64_sextet_count_per_group-1; sextet_index >= 0; --sextet_index) {
			const int sextet_offset= sextet_index * b64_sextet_size;
			if (static_cast<int>(sextet_offset + b64_sextet_size) > lo_used_octet_offset) {
				out_buffer[output_index++]= b64_index_table[(joined & (b64_sextet << sextet_offset)) >> sextet_offset];
			} else {
				out_buffer[output_index++]= b64_padding;
			}
		}
	}

	assert(output_index == *out_len);

	out_buffer[*out_len]= '\0';
	return out_buffer;
}
