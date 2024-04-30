#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "heat-3d.hpp"

// autotuning
#include <noarr/tuning/formatters/opentuner_formatter.hpp>
#ifdef NOARR_TUNING_DEFINES_FILE
#include STRINGIFY(NOARR_TUNING_DEFINES_FILE)
#endif

using num_t = DATA_TYPE;

namespace {

constexpr auto i_vec = noarr::vector<'i'>();
constexpr auto j_vec = noarr::vector<'j'>();
constexpr auto k_vec = noarr::vector<'k'>();

struct tuning {
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("heat-3d"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 'v'>, 6);
	NOARR_TUNE_PAR(block_k, noarr::tuning::mapped_range, into_pow2_blocks<'k', 'K', 'k', 'u'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_j ^ *block_k);

	NOARR_TUNE_PAR(loop_order, noarr::tuning::mapped_permutation, xor_fold, noarr::hoist<'k'>(), noarr::hoist<'j'>(), noarr::hoist<'i'>());
	NOARR_TUNE_PAR(grid_order, noarr::tuning::mapped_permutation, xor_fold, noarr::hoist<'K'>(), noarr::hoist<'J'>(), noarr::hoist<'I'>());

	NOARR_TUNE_CONST(order, *loop_order ^ *grid ^ *grid_order);

	NOARR_TUNE_PAR(a_layout, noarr::tuning::mapped_permutation, xor_fold, k_vec, j_vec, i_vec);
	NOARR_TUNE_PAR(b_layout, noarr::tuning::mapped_permutation, xor_fold, k_vec, j_vec, i_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(auto A, auto B) {
	// A: i x j x k
	// B: i x j x k
	using namespace noarr;

	auto n = A | get_length<'i'>();

	traverser(A, B) | [=](auto state) {
		auto [i, j, k] = get_indices<'i', 'j', 'k'>(state);
		A[state] = B[state] = (num_t) (i + j + (n - k)) * 10 / n;
	};
}

// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_heat_3d(std::size_t tsteps, auto A, auto B, Order order = {}) {
	// A: i x j x k
	// B: i x j x k
	using namespace noarr;

	auto trav = traverser(A, B) ^ bcast<'t'>(tsteps);

	#pragma scop
	trav ^ symmetric_spans<'i', 'j', 'k'>(A, 1, 1, 1) ^ order | for_dims<'t'>([=](auto inner) {
		inner | [=](auto state) {
			B[state] =
				(num_t).125 * (A[state - idx<'i'>(1)] -
				               2 * A[state] +
				               A[state + idx<'i'>(1)]) +
				(num_t).125 * (A[state - idx<'j'>(1)] -
				               2 * A[state] +
				               A[state + idx<'j'>(1)]) +
				(num_t).125 * (A[state - idx<'k'>(1)] -
				               2 * A[state] +
				               A[state + idx<'k'>(1)]) +
				A[state];
		};

		inner | [=](auto state) {
			A[state] =
				(num_t).125 * (B[state - idx<'i'>(1)] -
				               2 * B[state] +
				               B[state + idx<'i'>(1)]) +
				(num_t).125 * (B[state - idx<'j'>(1)] -
				               2 * B[state] +
				               B[state + idx<'j'>(1)]) +
				(num_t).125 * (B[state - idx<'k'>(1)] -
				               2 * B[state] +
				               B[state + idx<'k'>(1)]) +
				B[state];
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

	auto set_lengths = noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n) ^ noarr::set_length<'k'>(n);

	// data
	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ set_lengths);
	auto B = noarr::bag(noarr::scalar<num_t>() ^ *tuning.b_layout ^ set_lengths);

	// initialize data
	init_array(A.get_ref(), B.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_heat_3d(t, A.get_ref(), B.get_ref(), *tuning.order);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, A.get_ref() ^ noarr::reorder<'i', 'j', 'k'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
