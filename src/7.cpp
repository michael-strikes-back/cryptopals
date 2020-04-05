
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
	#include <openssl/ssl.h>
}

#include "shared.hpp"

void init_ssl() {
	SSL_load_error_strings();
	SSL_library_init();
}

void main_7(int argc, const char **argv) {
	init_ssl();
}
