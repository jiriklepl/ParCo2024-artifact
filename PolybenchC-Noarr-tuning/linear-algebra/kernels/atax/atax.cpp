#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "atax.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("atax"));

	NOARR_TUNE_PAR(block_i1, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j1, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_PAR(block_i2, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j2, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_CONST(grid1, *block_i1 ^ *block_j1);
	NOARR_TUNE_CONST(grid2, *block_i2 ^ *block_j2);

	NOARR_TUNE_PAR(loop_order1, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());
	NOARR_TUNE_PAR(loop_order2, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());

	NOARR_TUNE_PAR(order1, noarr::tuning::choice,
		*loop_order1 ^ *grid1 ^ noarr::hoist<'I', 'J'>(),
		*loop_order1 ^ *grid1 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(order2, noarr::tuning::choice,
		*loop_order2 ^ *grid2 ^ noarr::hoist<'I', 'J'>(),
		*loop_order2 ^ *grid2 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(c_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(auto A, auto x) {
	// A: i x j
	// x: j
	using namespace noarr;

	auto ni = A | get_length<'i'>();
	auto nj = A | get_length<'j'>();

	traverser(x) | [=](auto state) {
		auto j = get_index<'j'>(state);
		x[state] = 1 + j / (num_t)nj;
	};

	traverser(A) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);
		A[state] = (num_t)((i + j) % nj) / (5 * ni);
	};
}

// computation kernel
template<class Order1 = noarr::neutral_proto, class Order2 = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_atax(auto A, auto x, auto y, auto tmp, Order1 order1 = {}, Order2 order2 = {}) {
	// A: i x j
	// x: j
	// y: j
	// tmp: i
	using namespace noarr;

	#pragma scop
	traverser(y) | [=](auto state) {
		y[state] = 0;
	};

	traverser(tmp) | [=](auto state) {
		tmp[state] = 0;
	};

	traverser(tmp, A, x) ^ order1 | [=](auto state) {
		tmp[state] += A[state] * x[state];
	};

	traverser(y, A, tmp) ^ order2 | [=](auto state) {
		y[state] += A[state] * tmp[state];
	};
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t ni = NI;
	std::size_t nj = NJ;

	// data
	auto A = noarr::make_bag(noarr::scalar<num_t>() ^ *tuning.c_layout ^ noarr::set_length<'i'>(ni) ^ noarr::set_length<'j'>(nj));

	auto x = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(nj));
	auto y = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(nj));

	auto tmp = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(ni));

	// initialize data
	init_array(A.get_ref(), x.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_atax(A.get_ref(), x.get_ref(), y.get_ref(), tmp.get_ref(), *tuning.order1, *tuning.order2);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, y);
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
