
#include <cassert>
#include <iostream>
#include "main.hpp"

int main(const int argc, const char **argv) {
	if (argc < 2 || argv[1] == nullptr || argv[1][0] != '-' ||
		argv[1][1] < '2' || argv[1][1] > '6' || argv[1][2] != '\0') {
		fputs("Option required. [-2 -3 -4 -5 -6] <args>", stderr);
		fputc('\n', stderr);
		return 1;
	}

	const char **sub_argv= argv + 2;
	const int sub_argc= argc - 2;

	switch (argv[1][1]) {
		case '2': main_2(sub_argc, sub_argv); break;
		case '3': main_3(sub_argc, sub_argv); break;
		case '4': main_4(sub_argc, sub_argv); break;
		case '5': main_5(sub_argc, sub_argv); break;
		case '6': main_6(sub_argc, sub_argv); break;
		default: assert(0);
	}

	return 0;
}
