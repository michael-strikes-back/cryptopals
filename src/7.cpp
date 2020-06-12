
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	#include <openssl/evp.h>
	#include <openssl/err.h>
	#include <openssl/ssl.h>
#else
#error "not bothering to support newer versions than OpenSSL 1.1.0"
#endif // OPENSSL_VERSION_NUMBER

#include "shared.hpp"
#include "main.hpp"

static const size_t read_chunk_size= 1024;
// Allow enough space in output buffer for additional block
byte_t outbuf[read_chunk_size + EVP_MAX_BLOCK_LENGTH];

void ssl_init() {
	SSL_load_error_strings();
	SSL_library_init();
}

void ssl_handle_errors() {
	ERR_print_errors_fp(stderr);
	abort();
}

template<>
void problem<7>(int argc, const char **argv) {

	if (argc != 2) {
		fputs("Usage: -7 [in file] [out file]\n", stderr);
		return;
	}

	ssl_init();

	EVP_CIPHER_CTX *ctx;
	// $NOTE: typically this wouldn't be hard-coded
	const byte_t key[]= "YELLOW SUBMARINE";
	int plaintext_len;
	size_t enciphered_len;
	FILE* outf;
	byte_t *enciphered_it;
	byte_t *enciphered_begin;
	byte_t *enciphered_end;

	// first decode entire input file into base64.
	enciphered_begin= get_enciphered_text_from_base64_file(argv[0], &enciphered_len);
	if (enciphered_begin==nullptr) {
		// already messaged
		return;
	}

	ctx= EVP_CIPHER_CTX_new();
	if (nullptr == ctx) {
		ssl_handle_errors();
	}
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr)) {
		ssl_handle_errors();
	}
	assert(16 == EVP_CIPHER_CTX_key_length(ctx));

	outf = fopen(argv[1], "wb");
	if (outf == nullptr) {
		/* Error */
		return;
	}

	enciphered_it= enciphered_begin;
	enciphered_end= enciphered_begin + enciphered_len;
	for (; enciphered_it + read_chunk_size < enciphered_end; enciphered_it+= read_chunk_size) {
		if (1 != EVP_DecryptUpdate(ctx, outbuf, &plaintext_len, enciphered_it, read_chunk_size)) {
			ssl_handle_errors();
		}
		fwrite(outbuf, 1, plaintext_len, outf);
	}

	if (enciphered_it < enciphered_end) {
		if (1 != EVP_DecryptUpdate(ctx, outbuf, &plaintext_len, enciphered_it, enciphered_end - enciphered_it)) {
			ssl_handle_errors();
		}
		fwrite(outbuf, 1, plaintext_len, outf);
	}

	if (1 != EVP_DecryptFinal_ex(ctx, outbuf, &plaintext_len)) {
		ssl_handle_errors();
	}

	fwrite(outbuf, 1, plaintext_len, outf);
	fclose(outf);

	EVP_CIPHER_CTX_free(ctx);
	free(enciphered_begin);
}
