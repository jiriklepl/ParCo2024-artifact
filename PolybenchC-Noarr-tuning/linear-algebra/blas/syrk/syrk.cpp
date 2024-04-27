#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "syrk.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("syrk"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_k, noarr::tuning::mapped_range, into_pow2_blocks<'k', 'K', 'k', 'u'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_k);

	NOARR_TUNE_PAR(loop_order, noarr::tuning::choice, noarr::hoist<'i'>(), noarr::hoist<'k'>());

	NOARR_TUNE_PAR(order, noarr::tuning::choice,
		*loop_order ^ *grid ^ noarr::hoist<'I', 'K'>(),
		*loop_order ^ *grid ^ noarr::hoist<'K', 'I'>());

	NOARR_TUNE_PAR(c_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);
	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, k_vec ^ i_vec, i_vec ^ k_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(num_t &alpha, num_t &beta, auto C, auto A) {
	// C: i x j
	// A: i x k
	using namespace noarr;

	alpha = (num_t)1.5;
	beta = (num_t)1.2;

	auto ni = C | get_length<'i'>();
	auto nk = A | get_length<'k'>();

	traverser(A) | [=](auto state) {
		auto [i, k] = get_indices<'i', 'k'>(state);

		A[state] = (num_t)((i * k + 1) % ni) / ni;
	};

	traverser(C) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);

		C[state] = (num_t)((i * j + 2) % nk) / nk;
	};
}

// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_syrk(num_t alpha, num_t beta, auto C, auto A, Order order = {}) {
	// C: i x j
	// A: i x k
	using namespace noarr;

	auto A_renamed = A ^ rename<'i', 'j'>();

	#pragma scop
	traverser(C) | for_dims<'i'>([=](auto inner) {
		inner ^ span<'j'>(get_index<'i'>(inner) + 1) | [=](auto state) {
			C[state] *= beta;
		};
	});

	planner(C, A) ^ for_each([=](auto state) {
		C[state] += alpha * A[state] * A_renamed[state];
	}) ^ for_dims<'i'>([](auto inner) {
		inner ^ span<'j'>(get_index<'i'>(inner) + 1) | planner_execute();
	}) ^ order | planner_execute();
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t ni = NI;
	std::size_t nk = NK;

	// data
	num_t alpha;
	num_t beta;

	auto set_lengths = noarr::set_length<'i'>(ni) ^ noarr::set_length<'k'>(nk) ^ noarr::set_length<'j'>(ni);

	auto C = noarr::make_bag(noarr::scalar<num_t>() ^ *tuning.c_layout ^ set_lengths);
	auto A = noarr::make_bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ set_lengths);

	// initialize data
	init_array(alpha, beta, C.get_ref(), A.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_syrk(alpha, beta, C.get_ref(), A.get_ref(), *tuning.order);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, C.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
