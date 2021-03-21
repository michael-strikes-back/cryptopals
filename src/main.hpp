#pragma once

#ifndef MAIN_HPP
#define MAIN_HPP

template<unsigned prob_no>
struct problem_node {
	static constexpr unsigned this_problem_number= prob_no;
	static constexpr unsigned next_problem_number= this_problem_number-1;

	static void try_invoke(
		unsigned problem_index,
		int argc,
		const char **argv);

	// to implement a new problem, specialize this method and bump max_problem_number in main.cpp
	static void invoke(
		int argc,
		const char **argv);
};

#endif // MAIN_HPP

