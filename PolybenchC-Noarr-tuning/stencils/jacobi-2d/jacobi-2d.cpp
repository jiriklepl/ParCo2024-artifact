#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "jacobi-2d.hpp"

// autotuning
#include <noarr/tuning/formatters/opentuner_formatter.hpp>
#ifdef NOARR_TUNING_DEFINES_FILE
#include STRINGIFY(NOARR_TUNING_DEFINES_FILE)
#endif

using num_t = DATA_TYPE;

namespace {

constexpr auto i_vec = noarr::vector<'i'>();
constexpr auto j_vec = noarr::vector<'j'>();

struct tuning {
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("jacobi-2d"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 'a'>, 6);
	NOARR_TUNE_PAR(block_j, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 'b'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_j);

	NOARR_TUNE_PAR(loop_order, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());

	NOARR_TUNE_PAR(order, noarr::tuning::choice,
		*loop_order ^ *grid ^ noarr::hoist<'I', 'J'>(),
		*loop_order ^ *grid ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);
	NOARR_TUNE_PAR(b_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;


// initialization function
void init_array(auto A, auto B) {
	// A: i x j
	// B: i x j
	using namespace noarr;

	auto n = A | get_length<'i'>();

	traverser(A, B) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);

		A[state] = ((num_t)i * (j + 2) + 2) / n;
		B[state] = ((num_t)i * (j + 3) + 3) / n;
	};
}


// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_jacobi_2d(std::size_t tsteps, auto A, auto B, Order order = {}) {
	// A: i x j
	// B: i x j
	using namespace noarr;

	auto trav = traverser(A, B) ^ bcast<'t'>(tsteps);

	#pragma scop
	trav ^ symmetric_spans<'i', 'j'>(A, 1, 1) ^ order | for_dims<'t'>([=](auto inner) {
		inner | [=](auto state) {
			B[state] = (num_t).2 * (
				A[state] +
				A[state - idx<'j'>(1)] +
				A[state + idx<'j'>(1)] +
				A[state + idx<'i'>(1)] +
				A[state - idx<'i'>(1)]);
		};

		inner | [=](auto state) {
			A[state] = (num_t).2 * (
				B[state] +
				B[state - idx<'j'>(1)] +
				B[state + idx<'j'>(1)] +
				B[state + idx<'i'>(1)] +
				B[state - idx<'i'>(1)]);
		};
	});
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t n = N;
	std::size_t t = TSTEPS;

	// data
	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n));
	auto B = noarr::bag(noarr::scalar<num_t>() ^ *tuning.b_layout ^ noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n));

	// initialize data
	init_array(A.get_ref(), B.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_jacobi_2d(t, A.get_ref(), B.get_ref(), *tuning.order);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, A.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
