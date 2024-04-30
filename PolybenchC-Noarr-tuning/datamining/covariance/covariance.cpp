#include <chrono>
#include <iomanip>
#include <iostream>

#include <noarr/traversers.hpp>

#include "defines.hpp"
#include "utility.hpp"
#include "covariance.hpp"

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
	NOARR_TUNE_BEGIN(MAKE_OPENTUNER_FORMATTER("covariance"));

	NOARR_TUNE_PAR(block_i, noarr::tuning::mapped_range, into_pow2_blocks<'i', 'I', 'i', 's'>, 6);
	NOARR_TUNE_PAR(block_k, noarr::tuning::mapped_range, into_pow2_blocks<'k', 'K', 'k', 'u'>, 6);

	NOARR_TUNE_CONST(grid, *block_i ^ *block_k);

	NOARR_TUNE_PAR(order, noarr::tuning::choice,
		*grid ^ noarr::hoist<'I', 'K'>(),
		*grid ^ noarr::hoist<'K', 'I'>());

	NOARR_TUNE_PAR(data_layout, noarr::tuning::choice, j_vec ^ k_vec, k_vec ^ j_vec);
	NOARR_TUNE_PAR(cov_layout, noarr::tuning::choice, j_vec ^ i_vec, i_vec ^ j_vec);

	NOARR_TUNE_END();
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
		inner ^ shift<'j'>(get_index<'i'>(inner)) | [=](auto inner) {
			cov[inner] = 0;
		};
	});

	planner(data, cov, mean) ^ for_each([=](auto state) {
		cov[state] += data[state] * data_ki[state];
	}) ^ for_sections<'i'>([](auto inner) {
		inner ^ shift<'j'>(get_index<'i'>(inner)) | planner_execute();
	}) ^ hoist<'i', 'j', 'k'>() | planner_execute();

	traverser(cov, cov_ji) | for_dims<'i'>([=](auto inner) {
		inner ^ shift<'j'>(get_index<'i'>(inner)) | [=](auto state) {
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
	auto data = noarr::bag(noarr::scalar<num_t>() ^ *tuning.data_layout ^ set_lengths);
	auto cov = noarr::bag(noarr::scalar<num_t>() ^ *tuning.cov_layout ^ set_lengths);
	auto mean = noarr::bag(noarr::scalar<num_t>() ^ noarr::vector<'j'>(nj));

	// initialize data
	init_array(float_n, data.get_ref());

	auto start = std::chrono::high_resolution_clock::now();

	// run kernel
	kernel_covariance(float_n, data.get_ref(), cov.get_ref(), mean.get_ref(), *tuning.order);

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
