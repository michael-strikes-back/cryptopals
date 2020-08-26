
#include <cassert>
#include <iostream>
#include "main.hpp"

constexpr int max_problem_number= 8;

template<int prob_no>
void problem_node<prob_no>::try_invoke(
		int problem_index,
		int argc,
		const char **argv) {

	if (problem_index == this_problem_number) {
		invoke(argc, argv);
		return;
	}
	problem_node<next_problem_number>::try_invoke(problem_index, argc, argv);
}

template<> struct problem_node<0> {
	static inline void try_invoke(int problem_index, int argc, const char **argv) {
		// we didn't find the problem_index
		assert(0);
	}
	static inline void invoke(int argc, const char **argv) {
		// unreachable
		assert(0);
	}
};

int main(const int argc, const char **argv) {
	
	if (argc < 2 || argv[1] == nullptr || argv[1][0] != '-' ||
		argv[1][1] < '1' || argv[1][1] > '8' || argv[1][2] != '\0') {
		fprintf(stderr, "Option required. [-1 .. -%d] <args>\n", max_problem_number);
		fputc('\n', stderr);
		return 1;
	}

	const char **sub_argv= argv + 2;
	const int sub_argc= argc - 2;

	const char problem_index= argv[1][1] - '0';
	assert(problem_index >= 1);

	problem_node<max_problem_number>::try_invoke(problem_index, sub_argc, sub_argv);

	return 0;
}
