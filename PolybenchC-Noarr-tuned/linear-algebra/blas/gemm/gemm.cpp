#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>

#include "defines.hpp"
#include "gemm.hpp"

using num_t = DATA_TYPE;

namespace {

constexpr auto i_vec =  noarr::vector<'i'>();
constexpr auto j_vec =  noarr::vector<'j'>();
constexpr auto k_vec =  noarr::vector<'k'>();

struct tuning {
	DEFINE_PROTO_STRUCT(block_i, noarr::strip_mine_dynamic<'i', 'I', 'i', 's'>(16));
	DEFINE_PROTO_STRUCT(block_j, noarr::neutral_proto());
	DEFINE_PROTO_STRUCT(block_k, noarr::strip_mine_dynamic<'k', 'K', 'k', 'u'>(2));

	DEFINE_PROTO_STRUCT(loop_order, noarr::hoist<'j'>() ^ noarr::hoist<'k'>() ^ noarr::hoist<'i'>());

	DEFINE_PROTO_STRUCT(order, loop_order ^ block_j ^ block_k ^ block_i);

	DEFINE_PROTO_STRUCT(c_layout, j_vec ^ i_vec);
	DEFINE_PROTO_STRUCT(a_layout, k_vec ^ i_vec);
	DEFINE_PROTO_STRUCT(b_layout, j_vec ^ k_vec);
} tuning;

// initialization function
void init_array(num_t &alpha, num_t &beta, auto C, auto A, auto B) {
	// C: i x j
	// A: i x k
	// B: k x j
	using namespace noarr;

	alpha = (num_t)1.5;
	beta = (num_t)1.2;

	traverser(C) | [=](auto state) {
		auto [i, j] = get_indices<'i', 'j'>(state);
		C[state] = (num_t)((i * j + 1) % (C | get_length<'i'>())) / (C | get_length<'i'>());
	};

	traverser(A) | [=](auto state) {
		auto [i, k] = get_indices<'i', 'k'>(state);
		A[state] = (num_t)(i * (k + 1) % (A | get_length<'k'>())) / (A | get_length<'k'>());
	};

	traverser(B) | [=](auto state) {
		auto [k, j] = get_indices<'k', 'j'>(state);
		B[state] = (num_t)(k * (j + 2) % (B | get_length<'j'>())) / (B | get_length<'j'>());
	};
}

// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_gemm(num_t alpha, num_t beta, auto C, auto A, auto B, Order order = {}) {
	// C: i x j
	// A: i x k
	// B: k x j
	using namespace noarr;

	#pragma scop
	traverser(C) | [=](auto state) {
		C[state] *= beta;
	};

	traverser(C, A, B) ^ order | [=](auto state) {
		C[state] += alpha * A[state] * B[state];
	};
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t ni = NI;
	std::size_t nj = NJ;
	std::size_t nk = NK;

	// input data
	num_t alpha;
	num_t beta;

	auto set_lengths = noarr::set_length<'i'>(ni) ^ noarr::set_length<'j'>(nj) ^ noarr::set_length<'k'>(nk);

	auto C = noarr::make_bag(noarr::scalar<num_t>() ^ tuning.c_layout ^ set_lengths);
	auto A = noarr::make_bag(noarr::scalar<num_t>() ^ tuning.a_layout ^ set_lengths);
	auto B = noarr::make_bag(noarr::scalar<num_t>() ^ tuning.b_layout ^ set_lengths);

	// initialize data
	init_array(alpha, beta, C.get_ref(), A.get_ref(), B.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_gemm(alpha, beta, C.get_ref(), A.get_ref(), B.get_ref(), tuning.order);

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
