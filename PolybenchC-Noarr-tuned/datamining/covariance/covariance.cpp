#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "covariance.hpp"

using num_t = DATA_TYPE;

namespace {

constexpr auto i_vec = noarr::vector<'i'>();
constexpr auto j_vec = noarr::vector<'j'>();
constexpr auto k_vec = noarr::vector<'k'>();

struct tuning {
	DEFINE_PROTO_STRUCT(block_i, noarr::strip_mine_dynamic<'i', 'I', 'i', 's'>(8));
	DEFINE_PROTO_STRUCT(block_k, noarr::strip_mine_dynamic<'k', 'K', 'k', 'u'>(16));

	DEFINE_PROTO_STRUCT(loop_order, noarr::hoist<'j'>() ^ noarr::hoist<'k'>() ^ noarr::hoist<'i'>());

	DEFINE_PROTO_STRUCT(order, loop_order ^ block_k ^ block_i);

	DEFINE_PROTO_STRUCT(data_layout, j_vec ^ k_vec);
	DEFINE_PROTO_STRUCT(cov_layout, j_vec ^ i_vec);
} tuning;

// initialization function
void init_array(num_t &float_n, auto data) {
	// data: k x j
	using namespace noarr;

	float_n = data | get_length<'k'>();

	traverser(data) | [=](auto state) {
		auto [k, j] = get_indices<'k', 'j'>(state);
		data[state] = (num_t)(k * j) / (data | get_length<'j'>());
	};
}

// computation kernel
template<class Order = noarr::neutral_proto>
[[gnu::flatten, gnu::noinline]]
void kernel_covariance(num_t float_n, auto data, auto cov, auto mean, Order order = {}) {
	// data: k x j
	// cov: i x j
	// mean: j
	using namespace noarr;

	auto cov_ji = cov ^ rename<'i', 'j', 'j', 'i'>();
	auto data_ki = data ^ rename<'j', 'i'>();

	#pragma scop
	traverser(mean) | [=](auto state) {
		mean[state] = 0;
	};

	traverser(data, mean) | [=](auto state) {
		mean[state] += data[state];
	};

	traverser(mean) | [=](auto state) {
		mean[state] /= float_n;
	};

	traverser(data, mean) | [=](auto state) {
		data[state] -= mean[state];
	};

	traverser(cov) | for_dims<'i'>([=](auto inner) {
		inner ^
			shift<'j'>(get_index<'i'>(inner.state())) |
			[=](auto state) {
				cov[state] = 0;
			};
	});

	planner(data, cov, mean) ^
		for_each([=](auto state) {
			cov[state] += data[state] * data_ki[state];
		}) ^
		for_sections<'i'>([](auto inner) {
			inner ^
				shift<'j'>(get_index<'i'>(inner.state())) |
				planner_execute();
		}) ^
		hoist<'k'>() ^
		hoist<'j'>() ^
		hoist<'i'>() ^
		order |
		planner_execute();

	traverser(cov, cov_ji) | for_dims<'i'>([=](auto inner) {
		inner ^
			shift<'j'>(get_index<'i'>(inner.state())) |
			[=](auto state) {
				cov[state] /= float_n - (num_t)1;
				cov_ji[state] = cov[state];
			};
	});
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t nk = NK;
	std::size_t nj = NJ;

	auto set_lengths = noarr::set_length<'k'>(nk) ^ noarr::set_length<'j'>(nj) ^ noarr::set_length<'i'>(nj);

	// data
	num_t float_n;
	auto data = noarr::bag(noarr::scalar<num_t>() ^ tuning.data_layout ^ set_lengths);
	auto cov = noarr::bag(noarr::scalar<num_t>() ^ tuning.cov_layout ^ set_lengths);
	auto mean = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(nj));

	// initialize data
	init_array(float_n, data.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_covariance(float_n, data.get_ref(), cov.get_ref(), mean.get_ref(), tuning.order);

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, cov.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
