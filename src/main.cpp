
#include <cassert>
#include <iostream>
#include "main.hpp"

int main(const int argc, const char **argv) {
	if (argc < 2 || argv[1] == nullptr || argv[1][0] != '-' ||
		argv[1][1] < '1' || argv[1][1] > '8' || argv[1][2] != '\0') {
		fputs("Option required. [-1 -2 -3 -4 -5 -6 -7 -8] <args>", stderr);
		fputc('\n', stderr);
		return 1;
	}

	const char **sub_argv= argv + 2;
	const int sub_argc= argc - 2;

	const char problem_index= argv[1][1] - '0';
	assert(problem_index >= 0);
	assert(problem_index <= 9);

	switch (problem_index) {
		case 1: problem<1>(sub_argc, sub_argv); break;
		case 2: problem<2>(sub_argc, sub_argv); break;
		case 3: problem<3>(sub_argc, sub_argv); break;
		case 4: problem<4>(sub_argc, sub_argv); break;
		case 5: problem<5>(sub_argc, sub_argv); break;
		case 6: problem<6>(sub_argc, sub_argv); break;
		case 7: problem<7>(sub_argc, sub_argv); break;
		case 8: problem<8>(sub_argc, sub_argv); break;
		default: assert(0);
	}

	return 0;
}
