
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

void ssl_init() {
	SSL_load_error_strings();
	SSL_library_init();
}

void ssl_handle_errors() {
	ERR_print_errors_fp(stderr);
	abort();
}

void main_7(int argc, const char **argv) {

	if (argc != 2) {
		fputs("Usage: -7 [in file] [out file]\n", stderr);
		return;
	}

	ssl_init();

	EVP_CIPHER_CTX *ctx;
	const byte_t key[]= "YELLOW SUBMARINE";
	/* Allow enough space in output buffer for additional block */
	byte_t outbuf[4096 + EVP_MAX_BLOCK_LENGTH];
	int outlen;
	int plaintext_len;
	size_t enciphered_len;
	FILE* outf;
	byte_t *enciphered;

	enciphered= get_enciphered_text_from_base64_file(argv[0], &enciphered_len);
	if (enciphered==nullptr) {
		// already messaged
		return;
	}

	if (nullptr == (ctx= EVP_CIPHER_CTX_new())) {
		ssl_handle_errors();
	}
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr)) {
		ssl_handle_errors();
	}
	assert(EVP_CIPHER_CTX_key_length(ctx) == 16);

	if (1 != EVP_DecryptUpdate(ctx, outbuf, &plaintext_len, enciphered, enciphered_len)) {
		ssl_handle_errors();
	}
	outlen= plaintext_len;

	if (1 != EVP_DecryptFinal_ex(ctx, outbuf + plaintext_len, &plaintext_len)) {
		ssl_handle_errors();
	}
	outlen+= plaintext_len;

	outf = fopen(argv[1], "wb");
	if (outf == nullptr) {
		/* Error */
		return;
	}

	fwrite(outbuf, 1, outlen, outf);
	fclose(outf);

	EVP_CIPHER_CTX_free(ctx);
}
