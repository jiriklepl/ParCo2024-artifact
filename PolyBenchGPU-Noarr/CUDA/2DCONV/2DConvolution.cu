#include <memory>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>
#include <noarr/structures/interop/cuda_traverser.cuh>

#include "common.hpp"
#include "defines.cuh"
#include "2DConvolution.cuh"

using num_t = DATA_TYPE;

namespace {

// initialize data
void init(auto A) {
	// A: i x j

	noarr::traverser(A).order(noarr::hoist<'i'>()).for_each([=](auto state) {
		A[state] = (float)rand() / (float)RAND_MAX;
	});
}

template<class inner_t, class A_t, class B_t>
__global__ void kernel_2dconv(inner_t inner, A_t A, B_t B) {
	// A: i x j
	// B: i x j
	using noarr::idx;

	num_t c11, c12, c13, c21, c22, c23, c31, c32, c33;

	c11 = +0.2;  c21 = +0.5;  c31 = -0.8;
	c12 = -0.3;  c22 = +0.6;  c32 = -0.9;
	c13 = +0.4;  c23 = +0.7;  c33 = +0.10;

	inner.template for_each<'s', 't'>([=](auto state) {
		auto [i, j] = noarr::get_indices<'i', 'j'>(state);

		if (i == 0 || j == 0)
			return;

		B[state] = c11 * A[state <'i'>(1) - idx<'j'>(1)] +
			c21 * A[state - idx<'i'>(1) + idx<'j'>(0)] +
			c31 * A[state - idx<'i'>(1) + idx<'j'>(1)] +
			c12 * A[state + idx<'i'>(0) - idx<'j'>(1)] +
			c22 * A[state + idx<'i'>(0) + idx<'j'>(0)] +
			c32 * A[state + idx<'i'>(0) + idx<'j'>(1)] +
			c13 * A[state + idx<'i'>(1) - idx<'j'>(1)] +
			c23 * A[state + idx<'i'>(1) + idx<'j'>(0)] +
			c33 * A[state + idx<'i'>(1) + idx<'j'>(1)];
	});
}

// run kernels
void run_2dconv(auto A, auto B) {
	// A: i x j
	// B: i x j
	auto trav = noarr::traverser(A, B)
		.order(noarr::span<'i'>(0, (A | noarr::get_length<'i'>()) - 1))
		.order(noarr::span<'j'>(0, (A | noarr::get_length<'j'>()) - 1))
		.order(noarr::into_blocks_dynamic<'i', 'I', 'i', 's'>(DIM_THREAD_BLOCK_Y))
		.order(noarr::into_blocks_dynamic<'j', 'J', 'j', 't'>(DIM_THREAD_BLOCK_X))
		;

	noarr::cuda_threads<'J', 'j', 'I', 'i'>(trav)
		.simple_run(kernel_2dconv, 0, A, B);

	CUCH(cudaGetLastError()); // check for configuration errors
	CUCH(cudaDeviceSynchronize()); // join, check for execution errors
}

class experiment : public virtual_experiment {
	template<class A, class B>
	struct experiment_data : public virtual_data {
		A a;
		B b;

		experiment_data(A a, B b)
			: a(std::move(a)), b(std::move(b)) { }

		void run() override {
			run_2dconv(a.get_device_ref(), b.get_device_ref());
		}

		void print_results(std::ostream& os) override {
			b.fetch_to_host();
			noarr::serialize_data(os, b.get_host_ref() ^ noarr::hoist<'i'>());
		}
	};

public:
	experiment() {
		// problem size
		std::size_t ni = NI;
		std::size_t nj = NJ;

		cudaInit();

		// data
		experiment_data new_data{
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'i'>(nj, ni)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'i'>(nj, ni))
		};

		init(new_data.a.get_host_ref());

		new_data.a.fetch_to_device();

		data = std::make_unique<decltype(new_data)>(std::move(new_data));
	}
};


} // namespace

REGISTER_EXPERIMENT(2DConvolution);
