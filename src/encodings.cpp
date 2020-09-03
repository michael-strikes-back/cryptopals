
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "encodings.hpp"
#include "macros.hpp"

//    private data

constexpr int octet_size= 8;
constexpr int sextet_size= 6;

constexpr char b64_padding= '=';

constexpr unsigned int b64_sextet_mask= 0x3f;
constexpr unsigned int b64_octet_mask= 0xff;
constexpr int b64_bits_per_character_group= 24;
constexpr int b64_octet_count_per_group= b64_bits_per_character_group/octet_size;
constexpr int b64_sextet_count_per_group= b64_bits_per_character_group/sextet_size;

//    public definitions

bool hex_decode(const char *str, size_t strn, uint8_t *out_bytes, size_t byten_max, size_t *out_byten) {
	// zero when not a valid nibble character
	// otherwise, negate to get the mapped value
	static constexpr char decode_table[]= {
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x10
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x20
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x30
		~0x0,~0x1,~0x2,~0x3,~0x4,~0x5,~0x6,~0x7,~0x8,~0x9,{},{},{},{},{},{}, // 0x40
		{},~0xA,~0xB,~0xC,~0xD,~0xE,~0xF,{},{},{},{},{},{},{},{},{},                 // 0x50
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                         // 0x60
		{},~0xa,~0xb,~0xc,~0xd,~0xe,~0xf,{},{},{},{},{},{},{},{},{},                 // 0x70
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
	static_assert(sizeof(char) == 1);

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

		char hi_nibble= decode_table[static_cast<unsigned char>(str[i])];
		char lo_nibble= decode_table[static_cast<unsigned char>(str[i+1])];

		// check for success
		success= hi_nibble && lo_nibble;
		hi_nibble= ~hi_nibble;
		lo_nibble= ~lo_nibble;

		out_bytes[i/2]= (hi_nibble << 4) | lo_nibble;
	}

	*out_byten= i/2;

	return success;
}

size_t hex_encode(const uint8_t *bytes, size_t bytesn, char *out_str, size_t str_max) {

	// I was lazy and made assumptions when I wrote this.
	static_assert(sizeof(char)==1);

	assert(str_max >= bytesn * 2 + 1);
	constexpr char encode_table[]= "0123456789abcdef";
	static_assert(COUNT_OF(encode_table)-1 == 0x10);

	size_t i= 0;

	// unroll for the bulk of the data
	const size_t bytesn_aligned= bytesn & ~0xfull;
	for (; i < bytesn_aligned; i+= 0x10) {
		for (int offset= 0; offset < 0x10; ++offset) {
			out_str[(i+offset)*2]= encode_table[bytes[(i+offset)]>>4];
			out_str[(i+offset)*2 + 1]= encode_table[bytes[(i+offset)]&0xf];
		}
	}

	// remainder
	for (; i < bytesn; ++i) {
		out_str[i*2]= encode_table[bytes[i]>>4];
		out_str[i*2 + 1]= encode_table[bytes[i]&0xf];
	}

	out_str[i*2]= '\0';

	// return the length of the encoded string
	return i*2;
}

bool base64_decode(const char *const encoded_data, size_t encoded_len_raw, uint8_t **const out_bytes, size_t *const out_len) {

	// zero when not a valid character for the mapped sextet.
	// otherwise, negate to get the correct value
	static constexpr int8_t decode_table[]= {
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0x10
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0x20
		{},{},{},{},{},{},{},{},{},{},{},~0x3e,{},{},{},~0x3f,                                        // 0x30
		~0x34,~0x35,~0x36,~0x37,~0x38,~0x39,~0x3a,~0x3b,~0x3c,~0x3d,{},{},{},{},{},{},                // 0x40
		{},~0x0,~0x1,~0x2,~0x3,~0x4,~0x5,~0x6,~0x7,~0x8,~0x9,~0xa,~0xb,~0xc,~0xd,~0xe,                // 0x50
		~0xf,~0x10,~0x11,~0x12,~0x13,~0x14,~0x15,~0x16,~0x17,~0x18,~0x19,{},{},{},{},{},              // 0x60
		{},~0x1a,~0x1b,~0x1c,~0x1d,~0x1e,~0x1f,~0x20,~0x21,~0x22,~0x23,~0x24,~0x25,~0x26,~0x27,~0x28, // 0x70
		~0x29,~0x2a,~0x2b,~0x2c,~0x2d,~0x2e,~0x2f,~0x30,~0x31,~0x32,~0x33,{},{},{},{},{},             // 0x80
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0x90
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xa0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xb0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xc0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xd0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xe0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0xf0
		{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},                                              // 0x100
	};
	static_assert(COUNT_OF(decode_table) == 0x100);
	static_assert(sizeof(char) == 1);

	assert(encoded_data);
	assert(out_len);
	assert(out_bytes);

	size_t encoded_len= 0;
	size_t encoded_raw_last_group_begin;
	size_t encoded_index_raw;

	// $FIXME: SLOW!
	{
		size_t first_in_group= -1;
		int group_count= 0;
		
		for (encoded_index_raw= 0;
			encoded_index_raw < encoded_len_raw && encoded_data[encoded_index_raw]!='\0';
			++encoded_index_raw) {

			if (encoded_data[encoded_index_raw] == b64_padding || !isspace(encoded_data[encoded_index_raw])) {
				encoded_len++;

				group_count= (group_count + 1) % b64_sextet_count_per_group;
				if (group_count == 1) {
					first_in_group= encoded_index_raw;
				}
			}
		}
		encoded_raw_last_group_begin= first_in_group;
	}

	*out_bytes= nullptr;
	*out_len= 0;

	// calculate the decoded length
	if ((encoded_len % b64_sextet_count_per_group) != 0) {
		// $TODO messaging
		return false;
	}

	if (encoded_len == 0) {
		// nothing to decode
		return true;
	}

	*out_bytes= new uint8_t[encoded_len / b64_sextet_count_per_group * b64_octet_count_per_group];
	assert(*out_bytes);

	bool success= true;

	size_t output_index= 0;

	for (encoded_index_raw= 0; success && encoded_index_raw < encoded_raw_last_group_begin;) {
		unsigned char usable_encoded_chars[b64_sextet_count_per_group];
		register uint32_t joined= 0;
		int32_t usable_encoded_char_count= 0;

		// read in the next block
		for (; usable_encoded_char_count < b64_sextet_count_per_group;) {
			const char encoded_char= encoded_data[encoded_index_raw++];
			if (!isspace(encoded_char)) {
				usable_encoded_chars[usable_encoded_char_count++]= static_cast<unsigned char>(encoded_char);
			}
		}

		for (int32_t sextet_index= 0; sextet_index < b64_sextet_count_per_group; ++sextet_index) {
			const int8_t sextet_value= decode_table[usable_encoded_chars[sextet_index]];
			success= success && sextet_value;
			// in a given group, the hi b64_sextet_mask is the first char, and lo b64_sextet_mask is the last char.
			joined|= ~sextet_value << ((b64_sextet_count_per_group - 1 - sextet_index) * sextet_size);
		}
		for (int32_t octet_index= b64_octet_count_per_group - 1; octet_index >= 0; --octet_index) {
			const int32_t octet_offset= octet_index * octet_size;
			const int32_t octet_value= (joined & (b64_octet_mask << octet_offset)) >> octet_offset;
			(*out_bytes)[output_index++]= octet_value;
		}
	}

	// one more group to decode
	{
		uint8_t decoded_sextets[b64_sextet_count_per_group];
		register uint32_t joined= 0;
		int32_t usable_encoded_char_count= 0;
		int32_t lo_used_sextet_offset= 0;

		// read in the next block
		for (; usable_encoded_char_count < b64_sextet_count_per_group;) {
			const char encoded_char= encoded_data[encoded_index_raw++];
			if (encoded_char == b64_padding) {
				decoded_sextets[usable_encoded_char_count++]= 0;
			} else if (!isspace(encoded_char)) {
				const int8_t sextet_value= decode_table[static_cast<unsigned char>(encoded_char)];
				success= success && sextet_value;
				lo_used_sextet_offset= (b64_sextet_count_per_group - usable_encoded_char_count - 1) * sextet_size;
				decoded_sextets[usable_encoded_char_count++]= ~sextet_value;
			}
		}

		// in a given group, the hi b64_sextet_mask is the first char, and lo b64_sextet_mask is the last char.
		for (int32_t sextet_index= b64_sextet_count_per_group-1; sextet_index >= 0; --sextet_index) {
			const int8_t sextet_value= decoded_sextets[b64_sextet_count_per_group - sextet_index - 1];
			const int32_t sextet_offset= sextet_index * sextet_size;
			joined|= sextet_value << sextet_offset;
		}
		for (int32_t octet_index= b64_octet_count_per_group - 1; octet_index >= 0; --octet_index) {
			const int32_t octet_offset= octet_index * octet_size;
			const int32_t octet_value= (joined & (b64_octet_mask << octet_offset)) >> octet_offset;
			
			if (static_cast<int>(octet_offset) >= lo_used_sextet_offset) {
				(*out_bytes)[output_index++]= octet_value;
			}
		}
	}

	if (!success) {
		delete[] *out_bytes;
		*out_bytes= nullptr;
	} else {
		// correct length to remove padding
		*out_len= output_index;
	}

	// $TODO messaging
	return success;
}

char *base64_encode(const uint8_t *const data, size_t len, size_t *const out_len) {

	constexpr char encode_table[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static_assert(COUNT_OF(encode_table)-1 == 0100);

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
	assert(out_buffer);

	size_t offset_group= 0;
	size_t output_index= 0;

	for (; offset_group < len_truncated; offset_group+= b64_octet_count_per_group) {
		register uint32_t joined= 0;
		for (int32_t octet_index= 0; octet_index < b64_octet_count_per_group; ++octet_index) {
			// in a given group, the hi b64_sextet_mask is the first char, and lo b64_sextet_mask is the last char.
			const size_t data_index= offset_group + b64_octet_count_per_group - 1 - octet_index;
			joined|= data[data_index] << (octet_index*octet_size);
		}
		for (int32_t sextet_index= b64_sextet_count_per_group-1; sextet_index >= 0; --sextet_index) {
			const int32_t sextet_offset= sextet_index * sextet_size;
			out_buffer[output_index++]= encode_table[(joined & (b64_sextet_mask << sextet_offset)) >> sextet_offset];
		}
	}

	// calculate remainder with padding
	if (len != len_truncated) {
		register uint32_t joined= 0;
		int32_t lo_used_octet_offset= 0;

		for (uint32_t octet_index= 0; octet_index < b64_octet_count_per_group; ++octet_index) {
			// in a given group, the hi b64_sextet_mask is the first char, and lo b64_sextet_mask is the last char.
			const size_t data_index= offset_group + octet_index;
			if (data_index < len) {
				const int octet_offset= (b64_octet_count_per_group - octet_index - 1) * octet_size;
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
			const int sextet_offset= sextet_index * sextet_size;
			if (static_cast<int>(sextet_offset + sextet_size) > lo_used_octet_offset) {
				out_buffer[output_index++]= encode_table[(joined & (b64_sextet_mask << sextet_offset)) >> sextet_offset];
			} else {
				out_buffer[output_index++]= b64_padding;
			}
		}
	}

	assert(output_index == *out_len);

	out_buffer[*out_len]= '\0';
	return out_buffer;
}
