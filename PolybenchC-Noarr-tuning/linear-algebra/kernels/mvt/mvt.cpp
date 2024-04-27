#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "mvt.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("mvt"));

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

	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(auto x1, auto x2, auto y1, auto y2, auto A) {
	// x1: i
	// x2: i
	// y1: j
	// y2: j
	// A: i x j
	using namespace noarr;

	auto n = A | get_length<'i'>();

	auto y1_i = y1 ^ rename<'j', 'i'>();
	auto y2_i = y2 ^ rename<'j', 'i'>();

	traverser(x1, x2, y1_i, y2_i, A) | for_dims<'i'>([=](auto inner) {
		auto i = get_index<'i'>(inner);

		x1[inner] = (num_t)(i % n) / n;
		x2[inner] = (num_t)((i + 1) % n) / n;
		y1_i[inner] = (num_t)((i + 3) % n) / n;
		y2_i[inner] = (num_t)((i + 4) % n) / n;

		inner | [=](auto state) {
			auto j = get_index<'j'>(state);
			A[state] = (num_t)(i * j % n) / n;
		};
	});
}

// computation kernel
template<class Order1 = noarr::neutral_proto, class Order2 = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_mvt(auto x1, auto x2, auto y1, auto y2, auto A, Order1 order1 = {}, Order2 order2 = {}) {
	// x1: i
	// x2: i
	// y1: j
	// y2: j
	// A: i x j
	using namespace noarr;

	auto A_ji = A ^ rename<'i', 'j', 'j', 'i'>();

	#pragma scop
	traverser(x1, A, y1) ^ order1 |
		[=](auto state) {
			x1[state] += A[state] * y1[state];
		};

	traverser(x2, A_ji, y2) ^ order2 |
		[=](auto state) {
			x2[state] += A_ji[state] * y2[state];
		};
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t n = N;

	// data
	auto x1 = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));
	auto x2 = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));

	auto y1 = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(n));
	auto y2 = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(n));

	auto A = noarr::make_bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n));

	// initialize data
	init_array(x1.get_ref(), x2.get_ref(), y1.get_ref(), y2.get_ref(), A.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_mvt(x1.get_ref(), x2.get_ref(), y1.get_ref(), y2.get_ref(), A.get_ref(), *tuning.order1, *tuning.order2);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, x1);
		noarr::serialize_data(std::cout, x2);
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
