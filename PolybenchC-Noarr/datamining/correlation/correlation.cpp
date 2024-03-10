#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>

#include "defines.hpp"
#include "correlation.hpp"

using num_t = DATA_TYPE;

namespace {

constexpr auto i_vec =  noarr::vector<'i'>();
constexpr auto j_vec =  noarr::vector<'j'>();
constexpr auto k_vec =  noarr::vector<'k'>();

struct tuning {
	DEFINE_PROTO_STRUCT(data_layout, j_vec ^ k_vec);
	DEFINE_PROTO_STRUCT(corr_layout, j_vec ^ i_vec);
} tuning;

// initialization function
void init_array(num_t &float_n, auto data) {
	// data: k x j

	float_n = data | noarr::get_length<'k'>();

	noarr::traverser(data).for_each([=](auto state) {
		auto [k, j] = noarr::get_indices<'k', 'j'>(state);
		data[state] = (num_t)(k * j) / (data | noarr::get_length<'j'>()) + k;
	});
}

// computation kernel
[[gnu::flatten, gnu::noinline]]
void kernel_correlation(num_t float_n, auto data, auto corr, auto mean, auto stddev) {
	// data: k x j
	// corr: i x j
	// mean: j
	// stddev: j

	num_t eps = (num_t).1;

	auto corr_ji = corr ^ noarr::rename<'i', 'j', 'j', 'i'>();
	auto data_ki = data ^ noarr::rename<'j', 'i'>();

	auto ni = corr | noarr::get_length<'i'>();

	#pragma scop
	noarr::traverser(data, mean)
		.template for_dims<'j'>([=](auto inner) {
			auto state = inner.state();

			mean[state] = 0;

			inner.for_each([=](auto state) {
				mean[state] += data[state];
			});

			mean[state] /= float_n;
		});

	noarr::traverser(data, mean, stddev)
		.template for_dims<'j'>([=](auto inner) {
			auto state = inner.state();

			stddev[state] = 0;

			inner.for_each([=](auto state) {
				stddev[state] += (data[state] - mean[state]) * (data[state] - mean[state]);
			});

			stddev[state] /= float_n;
			stddev[state] = std::sqrt(stddev[state]);
			stddev[state] = stddev[state] <= eps ? (num_t)1.0 : stddev[state];
		});

	noarr::traverser(data, mean, stddev).for_each([=](auto state) {
		data[state] -= mean[state];
		data[state] /= std::sqrt(float_n) * stddev[state];
	});

	auto traverser = noarr::traverser(data, corr, data_ki, corr_ji);
	traverser
		.order(noarr::span<'i'>(0, ni - 1))
		.template for_dims<'i'>([=](auto inner) {
			auto state = inner.state();
			auto i = noarr::get_index<'i'>(state);

			corr[state & noarr::idx<'j'>(i)] = 1;

			inner
				.order(noarr::shift<'j'>(i + 1))
				.template for_dims<'j'>([=](auto inner) {
					auto state = inner.state();

					corr[state] = 0;

					inner.for_each([=](auto state) {
						corr[state] += data_ki[state] * data[state];
					});

					corr_ji[state] = corr[state];
				});
		});

	corr[noarr::idx<'i'>(ni - 1) & noarr::idx<'j'>(ni - 1)] = 1;
	#pragma endscop
}

} // namespace

int main(int argc, char *argv[]) {
	using namespace std::string_literals;

	// problem size
	std::size_t nk = NK;
	std::size_t nj = NJ;

	// data
	num_t float_n;
	auto data = noarr::make_bag(noarr::scalar<num_t>() ^ tuning.data_layout ^ noarr::set_length<'k'>(nk) ^ noarr::set_length<'j'>(nj));
	auto corr = noarr::make_bag(noarr::scalar<num_t>() ^ tuning.corr_layout ^ noarr::set_length<'i'>(nj) ^ noarr::set_length<'j'>(nj));
	auto mean = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::sized_vector<'j'>(nj));
	auto stddev = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::sized_vector<'j'>(nj));

	// initialize data
	init_array(float_n, data.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_correlation(float_n, data.get_ref(), corr.get_ref(), mean.get_ref(), stddev.get_ref());

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);

	// print results
	if (argc > 0 && argv[0] != ""s) {
		std::cout << std::fixed << std::setprecision(2);
		noarr::serialize_data(std::cout, corr.get_ref() ^ noarr::hoist<'i'>());
	}

	std::cerr << std::fixed << std::setprecision(6);
	std::cerr << duration.count() << std::endl;
}
