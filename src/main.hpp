#pragma once

#ifndef MAIN_HPP
#define MAIN_HPP

#ifdef NO
typedef void (problem_func*)(int argc, const char **argv) t_problem_func;

typedef t_problem_func[problem_count] t_problem_functions;
#endif // NO

template<unsigned prob_no>
struct problem_node {
	static constexpr unsigned this_problem_number= prob_no;
	static constexpr unsigned next_problem_number= this_problem_number-1;

	static void try_invoke(
		unsigned problem_index,
		int argc,
		const char **argv);

	static void invoke(
		int argc,
		const char **argv);
};

#endif // MAIN_HPP

