#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "3mm.hpp"

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
constexpr auto m_vec = noarr::vector<'m'>();

struct tuning {
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("3mm"));

	NOARR_TUNE_PAR(block_i1, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j1, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_PAR(block_j2, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);
	NOARR_TUNE_PAR(block_l2, noarr::tuning::mapped_range, into_pow2_blocks<'l', 'L', 'l', 'u'>, 6);

	NOARR_TUNE_PAR(block_i3, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_l3, noarr::tuning::mapped_range, into_pow2_blocks<'l', 'L', 'l', 'u'>, 6);

	NOARR_TUNE_CONST(grid1, *block_i1 ^ *block_j1);
	NOARR_TUNE_CONST(grid2, *block_j2 ^ *block_l2);
	NOARR_TUNE_CONST(grid3, *block_i3 ^ *block_l3);

	NOARR_TUNE_PAR(loop_order1, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'j'>());
	NOARR_TUNE_PAR(loop_order2, noarr::tuning::choice, noarr::hoist<'j'>(), noarr::hoist<'l'>());
	NOARR_TUNE_PAR(loop_order3, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'l'>());

	NOARR_TUNE_PAR(order1, noarr::tuning::choice,
		*loop_order1 ^ *grid1 ^ noarr::hoist<'I', 'J'>(),
		*loop_order1 ^ *grid1 ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(order2, noarr::tuning::choice,
		*loop_order2 ^ *grid2 ^ noarr::hoist<'J', 'L'>(),
		*loop_order2 ^ *grid2 ^ noarr::hoist<'L', 'J'>());
	
	NOARR_TUNE_PAR(order3, noarr::tuning::choice,
		*loop_order3 ^ *grid3 ^ noarr::hoist<'I', 'L'>(),
		*loop_order3 ^ *grid3 ^ noarr::hoist<'L', 'I'>());

	NOARR_TUNE_PAR(e_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);
	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, k_vec ^ i_vec, i_vec ^ k_vec);
	NOARR_TUNE_PAR(b_layout, noarr::tuning::choice, j_vec ^ k_vec, k_vec ^ j_vec);
	NOARR_TUNE_PAR(f_layout, noarr::tuning::choice, l_vec ^ j_vec, j_vec ^ l_vec);
	NOARR_TUNE_PAR(c_layout, noarr::tuning::choice, m_vec ^ j_vec, j_vec ^ m_vec);
	NOARR_TUNE_PAR(d_layout, noarr::tuning::choice, l_vec ^ m_vec, m_vec ^ l_vec);
	NOARR_TUNE_PAR(g_layout, noarr::tuning::choice, l_vec ^ i_vec, i_vec ^ l_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(auto A, auto B, auto C, auto D) {
	// A: i x k
	// B: k x j
	// C: j x m
	// D: m x l
	using namespace noarr;

	auto ni = A | get_length<'i'>();
	auto nj = B | get_length<'j'>();
	auto nk = A | get_length<'k'>();
	auto nl = D | get_length<'l'>();

	traverser(A) | [=](auto state) {
		auto [i, k] = get_indices<'i', 'k'>(state);
		A[state] = (num_t)((i * k + 1) % ni) / (5 * ni);
	};

	traverser(B) | [=](auto state) {
		auto [k, j] = get_indices<'k', 'j'>(state);
		B[state] = (num_t)((k * (j + 1) + 2) % nj) / (5 * nj);
	};

	traverser(C) | [=](auto state) {
		auto [j, m] = get_indices<'j', 'm'>(state);
		C[state] = (num_t)(j * (m + 3) % nl) / (5 * nl);
	};

	traverser(D) | [=](auto state) {
		auto [m, l] = get_indices<'m', 'l'>(state);
		D[state] = (num_t)((m * (l + 2) + 2) % nk) / (5 * nk);
	};
}

// computation kernel
template<class Order1 = noarr::neutral_proto, class Order2 = noarr::neutral_proto, class Order3 = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_3mm(auto E, auto A, auto B, auto F, auto C, auto D, auto G, Order1 order1 = {}, Order2 order2 = {}, Order3 order3 = {}) {
	// E: i x j
	// A: i x k
	// B: k x j
	// F: j x l
	// C: j x m
	// D: m x l
	// G: i x l
	using namespace noarr;

	constexpr auto madd = for_each_elem([](auto &&m, auto &&l, auto &&r) {
		m += l * r;
	});

	#pragma scop
	planner(E, A, B) ^ madd ^ for_dims<'i', 'j'>([=](auto inner) {
		E[inner] = 0;
		inner();
	}) ^ order1 | planner_execute();

	planner(F, C, D) ^ madd ^ for_dims<'j', 'l'>([=](auto inner) {
		F[inner] = 0;
		inner();
	}) ^ order2 | planner_execute();

	planner(G, E, F) ^ madd ^ for_dims<'i', 'l'>([=](auto inner) {
		G[inner] = 0;
		inner();
	}) ^ order3 | planner_execute();
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
	std::size_t nm = NM;

	auto set_lengths = noarr::set_length<'i'>(ni) ^ noarr::set_length<'j'>(nj) ^ noarr::set_length<'k'>(nk) ^ noarr::set_length<'l'>(nl) ^ noarr::set_length<'m'>(nm);

	// data
	auto E = noarr::bag(noarr::scalar<num_t>() ^ *tuning.e_layout ^ set_lengths);
	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ set_lengths);
	auto B = noarr::bag(noarr::scalar<num_t>() ^ *tuning.b_layout ^ set_lengths);

	auto F = noarr::bag(noarr::scalar<num_t>() ^ *tuning.f_layout ^ set_lengths);
	auto C = noarr::bag(noarr::scalar<num_t>() ^ *tuning.c_layout ^ set_lengths);
	auto D = noarr::bag(noarr::scalar<num_t>() ^ *tuning.d_layout ^ set_lengths);

	auto G = noarr::bag(noarr::scalar<num_t>() ^ *tuning.g_layout ^ set_lengths);

	// initialize data
	init_array(A.get_ref(), B.get_ref(), C.get_ref(), D.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_3mm(E.get_ref(), A.get_ref(), B.get_ref(),
		F.get_ref(), C.get_ref(), D.get_ref(),
		G.get_ref(),
		*tuning.order1, *tuning.order2, *tuning.order3);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, G.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
