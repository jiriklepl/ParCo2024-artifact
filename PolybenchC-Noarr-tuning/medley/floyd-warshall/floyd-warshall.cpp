#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "floyd-warshall.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("floyd-warshall"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);
	NOARR_TUNE_PAR(block_k, noarr::tuning::mapped_range, into_pow2_blocks<'k', 'K', 'k', 'u'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_j ^ *block_k);

	NOARR_TUNE_PAR(loop_order, noarr::tuning::mapped_permutation, xor_fold, noarr::hoist<'j'>(), noarr::hoist<'i'>(), noarr::hoist<'k'>());
	NOARR_TUNE_PAR(grid_order, noarr::tuning::mapped_permutation, xor_fold, noarr::hoist<'J'>(), noarr::hoist<'I'>(), noarr::hoist<'K'>());

	NOARR_TUNE_CONST(order, *loop_order ^ *grid ^ *grid_order);

	NOARR_TUNE_PAR(path_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(auto path) {
	// path: i x j
	using namespace noarr;

	traverser(path) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);

		path[state] = i * j % 7 + 1;

		if ((i + j) % 13 == 0 || (i + j) % 7 == 0 || (i + j) % 11 == 0)
			path[state] = 999;
	};
}


// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_floyd_warshall(auto path, Order order = {}) {
	// path: i x j
	using namespace noarr;

	auto path_start_k = path ^ rename<'i', 'k'>();
	auto path_end_k = path ^ rename<'j', 'k'>();

	#pragma scop
	traverser(path, path_start_k, path_end_k) ^ hoist<'k'>() ^ order |
		[=](auto state) {
			path[state] = std::min(path_start_k[state] + path_end_k[state], path[state]);
		};
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t n = N;

	// data
	auto path = noarr::make_bag(noarr::scalar<num_t>() ^ *tuning.path_layout ^ noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n));

	// initialize data
	init_array(path.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_floyd_warshall(path.get_ref(), *tuning.order);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, path.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
