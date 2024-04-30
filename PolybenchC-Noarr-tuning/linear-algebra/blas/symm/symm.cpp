#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "symm.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("symm"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_j, noarr::tuning::mapped_range, into_pow2_blocks<'j', 'J', 'j', 't'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_j);

	NOARR_TUNE_PAR(loop_order, noarr::tuning::choice,
		noarr::hoist<'i', 'j'>(),
		noarr::hoist<'j', 'i'>());

	NOARR_TUNE_PAR(order, noarr::tuning::choice,
		*loop_order ^ *grid ^ noarr::hoist<'I', 'J'>(),
		*loop_order ^ *grid ^ noarr::hoist<'J', 'I'>());

	NOARR_TUNE_PAR(c_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);
	NOARR_TUNE_PAR(a_layout, noarr::tuning::choice, k_vec ^ i_vec, i_vec ^ k_vec);
	NOARR_TUNE_PAR(b_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
} tuning;

// initialization function
void init_array(num_t &alpha, num_t &beta, auto C, auto A, auto B) {
	// C: i x j
	// A: i x k
	// B: i x j
	using namespace noarr;

	alpha = (num_t)1.5;
	beta = (num_t)1.2;

	auto ni = C | get_length<'i'>();
	auto nj = C | get_length<'j'>();

	traverser(C) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);
		C[state] = (num_t)((i + j) % 100) / ni;
		B[state] = (num_t)((nj + i - j) % 100) / ni;
	};

	traverser(A) | for_dims<'i'>([=](auto inner) {
		auto i = get_index<'i'>(inner);
		inner ^ span<'k'>(i + 1) | [=](auto state) {
			auto k = get_index<'k'>(state);
			A[state] = (num_t)((i + k) % 100) / ni;
		};

		inner ^ shift<'k'>(i + 1) | [=](auto state) {
			A[state] = -999;
		};
	});
}

// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_symm(num_t alpha, num_t beta, auto C, auto A, auto B, Order order = {}) {
	// C: i x j
	// A: i x k
	// B: i x j
	using namespace noarr;

	auto C_renamed = C ^ rename<'i', 'k'>();
	auto B_renamed = B ^ rename<'i', 'k'>();

	#pragma scop
	planner(C, A, B) ^ for_dims<'i', 'j'>([=](auto inner) {
		const auto i = get_index<'i'>(inner);

		num_t temp = 0;

		inner ^ span<'k'>(i) ^ for_each([=, &temp](auto state) {
			C_renamed[state] += alpha * B[state] * A[state];
			temp += B_renamed[state] * A[state];
		}) | planner_execute();

		C[inner] = beta * C[inner] + alpha * B[inner] * A[inner.state() & idx<'k'>(i)] + alpha * temp;
	}) ^ order | planner_execute();
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t ni = NI;
	std::size_t nj = NJ;

	// data
	num_t alpha;
	num_t beta;

	auto set_lengths = noarr::set_length<'i'>(ni) ^ noarr::set_length<'k'>(ni) ^ noarr::set_length<'j'>(nj);

	auto C = noarr::bag(noarr::scalar<num_t>() ^ *tuning.c_layout ^ set_lengths);
	auto A = noarr::bag(noarr::scalar<num_t>() ^ *tuning.a_layout ^ set_lengths);
	auto B = noarr::bag(noarr::scalar<num_t>() ^ *tuning.b_layout ^ set_lengths);

	// initialize data
	init_array(alpha, beta, C.get_ref(), A.get_ref(), B.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_symm(alpha, beta, C.get_ref(), A.get_ref(), B.get_ref(), *tuning.order);

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
