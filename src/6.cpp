
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "shared.hpp"
#include "main.hpp"

class c_skip_iterator : public c_string_iterator_interface {
public:
	c_skip_iterator(byte_t *source, size_t len, size_t offset, size_t skip_len)
		: m_offset(offset)
		, m_skip(skip_len)
		, m_source(source)
	{
		m_aligned_len= len - (len % skip_len) - (skip_len - offset);

		reset();
	}

	virtual bool has_next() { return m_it < m_aligned_len; }
	virtual char current() { return current_ref(); }
	virtual void next() { m_it+= m_skip; }

	inline char &current_ref() { return *reinterpret_cast<char*>(m_source + m_it); }
	inline void reset() { m_it= m_offset; }

private:
	size_t m_it;
	size_t m_offset;
	size_t m_skip;
	size_t m_aligned_len;
	byte_t *m_source;
};

byte_t get_best_key(c_skip_iterator &enciphered_it, int *out_best_score) {
	unsigned short best_key= 0;
	int best_score= -1;

	for (unsigned short key= 1; key < 256; ++key) {
		enciphered_it.reset();

		for (; enciphered_it.has_next(); enciphered_it.next()) {
			char &current_char= enciphered_it.current_ref();

			current_char^= key;
		}

		enciphered_it.reset();
		int score= score_plain_text(enciphered_it);

		if (score > best_score) {
			best_score= score;
			best_key= key;
		}

		// reverse
		enciphered_it.reset();
		for (; enciphered_it.has_next(); enciphered_it.next()) {
			char &current_char= enciphered_it.current_ref();

			current_char^= key;
		}
	}

	*out_best_score= best_score;

	assert(!(best_key&0xff00));
	return static_cast<byte_t>(best_key & 0xff);
}

template<>
void problem_node<6>::invoke(int argc, const char **argv) {

	if (argc != 1) {
		fputs("Usage: -6 [data file]\n", stderr);
		return;
	}

	size_t enciphered_len;
	byte_t *enciphered= get_enciphered_text_from_base64_file(argv[0], &enciphered_len);
	if (enciphered==nullptr) {
		// already messaged
		return;
	}

	unsigned short best_key_sizes[]= { 0, 0, 0 };
	{
		double lowest_edit_distances[]= { 900, 900, 900 };

		for(unsigned short key_size= 2; key_size <= 50; ++key_size) {
			double edit_distance= 0;
			const int k_distances_to_check= 4;
			for (int i= 0; i < k_distances_to_check; i++) {
				const byte_t *a= enciphered+key_size*i;
				const byte_t *b= enciphered+key_size*(i+1);

				edit_distance+= get_hamming_distance(a, b, key_size);
			}

			edit_distance/= k_distances_to_check * key_size;

			for (int ed_it= 0; ed_it < 3; ++ed_it) {
				if (edit_distance < lowest_edit_distances[ed_it]) {
					best_key_sizes[ed_it]= key_size;
					lowest_edit_distances[ed_it]= edit_distance;
					break;
				}
			}
		}
	}

	byte_t *key;
	{
		unsigned short max_key_size= 0;
		for (int key_size_it= 0; key_size_it < 3; ++key_size_it) {
			if (best_key_sizes[key_size_it]>max_key_size) {
				max_key_size= best_key_sizes[key_size_it];
			}
		}

		key= static_cast<byte_t *>(malloc(max_key_size));
	}

	for (int key_size_it= 0; key_size_it < 3; ++key_size_it) {
		unsigned short key_size= best_key_sizes[key_size_it];

		// I expect something completely useless to have a score close to 0
		const int k_good_enough_score= 3;
		int score= k_good_enough_score;

		for (unsigned short block_it= 0; score >= k_good_enough_score && block_it < key_size; ++block_it) {
			c_skip_iterator enciphered_it(
				enciphered,
				enciphered_len,
				block_it,
				key_size);

			byte_t best_key= get_best_key(enciphered_it, &score);
			key[block_it]= best_key;
		}

		if (score >= k_good_enough_score) {
			char out_file_name[256];
			sprintf(out_file_name, "6_%d.result", key_size);

			FILE *const outf= fopen(out_file_name, "wb");
			assert(outf);

			xor_repeating(enciphered, enciphered_len, key, key_size, enciphered, enciphered_len);

			fputs(reinterpret_cast<const char *>(enciphered), outf);

			fclose(outf);

			// reverse
			xor_repeating(enciphered, enciphered_len, key, key_size, enciphered, enciphered_len);

			printf("wrote '%s'\n", out_file_name);
		}
	}

	free(key);
	free(enciphered);
}

