#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "2mm.hpp"

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
constexpr auto l_vec = noarr::vector<'l'>();

struct tuning {
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("2mm"));

	NOARR_TUNE_PAR(block_i1, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j1, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_PAR(block_i2, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_l2, noarr::tuning::mapped_range, into_pow2_blocks<'l', 'L', 'l', 't'>, 6);

	NOARR_TUNE_CONST(grid1, *block_i1 ^ *block_j1);
	NOARR_TUNE_CONST(grid2, *block_i2 ^ *block_l2);

	NOARR_TUNE_PAR(loop_order1, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());
	NOARR_TUNE_PAR(loop_order2, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'l'>());

	NOARR_TUNE_PAR(order1, noarr::tuning::choice,
		*loop_order1 ^ *grid1 ^ noarr::hoist<'I', 'J'>(),
		*loop_order1 ^ *grid1 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(order2, noarr::tuning::choice,
		*loop_order2 ^ *grid2 ^ noarr::hoist<'I', 'L'>(),
		*loop_order2 ^ *grid2 ^ noarr::hoist<'L', 'I'>());

	NOARR_TUNE_PAR(tmp_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);
	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, k_vec ^ i_vec, i_vec ^ k_vec);
	NOARR_TUNE_PAR(b_layout, noarr::tuning::choice, j_vec ^ k_vec, k_vec ^ j_vec);
	NOARR_TUNE_PAR(c_layout, noarr::tuning::choice, l_vec ^ j_vec, j_vec ^ l_vec);
	NOARR_TUNE_PAR(d_layout, noarr::tuning::choice, l_vec ^ i_vec, i_vec ^ l_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(num_t &alpha, num_t &beta, auto A, auto B, auto C, auto D) {
	// tmp: i x j
	// A: i x k
	// B: k x j
	// C: j x l
	// D: i x l
	using namespace noarr;

	alpha = (num_t)1.5;
	beta = (num_t)1.2;

	auto ni = A | get_length<'i'>();
	auto nj = B | get_length<'j'>();
	auto nk = A | get_length<'k'>();
	auto nl = C | get_length<'l'>();

	traverser(A) | [=](auto state) {
		auto [i, k] = get_indices<'i', 'k'>(state);
		A[state] = (num_t)((i * k + 1) % ni) / ni;
	};

	traverser(B) | [=](auto state) {
		auto [k, j] = get_indices<'k', 'j'>(state);
		B[state] = (num_t)(k * (j + 1) % nj) / nj;
	};

	traverser(C) | [=](auto state) {
		auto [j, l] = get_indices<'j', 'l'>(state);
		C[state] = (num_t)((j * (l + 3) + 1) % nl) / nl;
	};

	traverser(D) | [=](auto state) {
		auto [i, l] = get_indices<'i', 'l'>(state);
		D[state] = (num_t)(i * (l + 2) % nk) / nk;
	};
}

// computation kernel
template<class Order1 = noarr::neutral_proto, class Order2 = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_2mm(num_t alpha, num_t beta, auto tmp, auto A, auto B, auto C, auto D, Order1 order1 = {}, Order2 order2 = {}) {
	// tmp: i x j
	// A: i x k
	// B: k x j
	// C: j x l
	// D: i x l
	using namespace noarr;

	#pragma scop
	planner(tmp, A, B) ^ for_each_elem([=](auto &&tmp, auto &&A, auto &&B) {
		tmp += alpha * A * B;
	}) ^ for_dims<'i', 'j'>([=](auto inner) {
		tmp[inner] = 0;
		inner();
	}) ^ order1 | planner_execute();

	planner(D, tmp, C) ^ for_each_elem([](auto &&D, auto &&tmp, auto &&C) {
		D += tmp * C;
	}) ^ for_dims<'i', 'l'>([=](auto inner) {
		D[inner] *= beta;
		inner();
	}) ^ order2 | planner_execute();
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t ni = NI;
	std::size_t nj = NJ;
	std::size_t nk = NK;
	std::size_t nl = NL;

	// data
	num_t alpha;
	num_t beta;

	auto set_lengths = noarr::set_length<'i'>(ni) ^ noarr::set_length<'j'>(nj) ^ noarr::set_length<'k'>(nk) ^ noarr::set_length<'l'>(nl);

	auto tmp = noarr::bag(noarr::scalar<num_t>() ^ *tuning.tmp_layout ^ set_lengths);

	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ set_lengths);
	auto B = noarr::bag(noarr::scalar<num_t>() ^ *tuning.b_layout ^ set_lengths);
	auto C = noarr::bag(noarr::scalar<num_t>() ^ *tuning.c_layout ^ set_lengths);

	auto D = noarr::bag(noarr::scalar<num_t>() ^ *tuning.d_layout ^ set_lengths);

	// initialize data
	init_array(alpha, beta, A.get_ref(), B.get_ref(), C.get_ref(), D.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_2mm(alpha, beta, tmp.get_ref(), A.get_ref(), B.get_ref(), C.get_ref(), D.get_ref(), *tuning.order1, *tuning.order2);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, D.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
