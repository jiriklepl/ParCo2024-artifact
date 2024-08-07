#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "gemver.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("gemver"));

	NOARR_TUNE_PAR(block_i1, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j1, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_PAR(block_i2, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j2, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_PAR(block_i3, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j3, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_CONST(grid1, *block_i1 ^ *block_j1);
	NOARR_TUNE_CONST(grid2, *block_i2 ^ *block_j2);
	NOARR_TUNE_CONST(grid3, *block_i3 ^ *block_j3);

	NOARR_TUNE_PAR(loop_order1, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());
	NOARR_TUNE_PAR(loop_order2, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());
	NOARR_TUNE_PAR(loop_order3, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());

	NOARR_TUNE_PAR(order1, noarr::tuning::choice,
		*loop_order1 ^ *grid1 ^ noarr::hoist<'I', 'J'>(),
		*loop_order1 ^ *grid1 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(order2, noarr::tuning::choice,
		*loop_order2 ^ *grid2 ^ noarr::hoist<'I', 'J'>(),
		*loop_order2 ^ *grid2 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(order3, noarr::tuning::choice,
		*loop_order3 ^ *grid3 ^ noarr::hoist<'I', 'J'>(),
		*loop_order3 ^ *grid3 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(num_t &alpha, num_t &beta, auto A, auto u1, auto v1, auto u2, auto v2, auto w, auto x, auto y, auto z) {
	// A: i x j
	// u1: i
	// v1: j
	// u2: i
	// v2: j
	// w: i
	// x: i
	// y: j
	// z: i
	using namespace noarr;

	alpha = (num_t)1.5;
	beta = (num_t)1.2;

	auto v1_i = v1 ^ rename<'j', 'i'>();
	auto v2_i = v2 ^ rename<'j', 'i'>();
	auto y_i = y ^ rename<'j', 'i'>();

	num_t fn = A | get_length<'i'>();

	traverser(A) | for_dims<'i'>([=](auto inner) {
		auto i = get_index<'i'>(inner);

		u1[inner] = i;
		u2[inner] = ((i + 1) / fn) / 2.0;
		v1_i[inner] = ((i + 1) / fn) / 4.0;
		v2_i[inner] = ((i + 1) / fn) / 6.0;
		y_i[inner] = ((i + 1) / fn) / 8.0;
		z[inner] = ((i + 1) / fn) / 9.0;
		x[inner] = 0.0;
		w[inner] = 0.0;

		inner | [=](auto state) {
			auto j = get_index<'j'>(state);
			A[state] = (num_t)(j * i % (A | get_length<'i'>())) / (A | get_length<'i'>());
		};
	});
}

// computation kernel
template<class Order1 = noarr::neutral_proto, class Order2 = noarr::neutral_proto, class Order3 = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_gemver(num_t alpha, num_t beta, auto A,
	auto u1, auto v1,
	auto u2, auto v2,
	auto w, auto x, auto y, auto z,
	Order1 order1 = {}, Order2 order2 = {}, Order3 order3 = {}) {
	// A: i x j
	// u1: i
	// v1: j
	// u2: i
	// v2: j
	// w: i
	// x: i
	// y: j
	// z: i
	using namespace noarr;

	auto A_ji = A ^ rename<'i', 'j', 'j', 'i'>();
	auto x_j = x ^ rename<'i', 'j'>();

	#pragma scop
	traverser(A, u1, u2, v1, v2) ^ order1 | [=](auto state) {
		A[state] = A[state] + u1[state] * v1[state] + u2[state] * v2[state];
	};

	traverser(x, A_ji, y) ^ order2 | [=](auto state) {
		x[state] = x[state] + beta * A_ji[state] * y[state];
	};

	traverser(x, z) | [=](auto state) {
		x[state] = x[state] + z[state];
	};

	traverser(A, w, x_j) ^ order3 | [=](auto state) {
		w[state] = w[state] + alpha * A[state] * x_j[state];
	};
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t n = N;

	// data
	num_t alpha;
	num_t beta;

	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ noarr::set_length<'i'>(n) ^ noarr::set_length<'j'>(n));
	auto u1 = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));
	auto v1 = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(n));
	auto u2 = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));
	auto v2 = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(n));
	auto w = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));
	auto x = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));
	auto y = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(n));
	auto z = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'i'>(n));

	// initialize data
	init_array(alpha, beta, A.get_ref(),
		u1.get_ref(), v1.get_ref(),
		u2.get_ref(), v2.get_ref(),
		w.get_ref(), x.get_ref(), y.get_ref(), z.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_gemver(alpha, beta, A.get_ref(),
		u1.get_ref(), v1.get_ref(),
		u2.get_ref(), v2.get_ref(),
		w.get_ref(), x.get_ref(), y.get_ref(), z.get_ref(),
		*tuning.order1, *tuning.order2, *tuning.order3);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, w);
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
