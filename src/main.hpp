#pragma once

#ifndef MAIN_HPP
#define MAIN_HPP

#ifdef NO
typedef void (problem_func*)(int argc, const char **argv) t_problem_func;

constexpr int problem_count= 8;
typedef t_problem_func[problem_count] t_problem_functions;
#endif // NO

template<int t_problem>
void problem(int argc, const char **argv);

#endif // MAIN_HPP

