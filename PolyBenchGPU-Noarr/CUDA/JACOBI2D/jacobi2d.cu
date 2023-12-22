#include <cuda_runtime_api.h>
#include <memory>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>
#include <noarr/structures/interop/cuda_traverser.cuh>

#include "common.hpp"
#include "defines.cuh"
#include "jacobi2d.cuh"

using num_t = DATA_TYPE;

namespace {

// initialize data
void init(auto A, auto B) {
	// A: i x j
	// B: i x j

	noarr::traverser(A, B).for_each([=](auto state){
		auto [i, j] = noarr::get_indices<'i', 'j'>(state);
		A[state] = ((num_t) i * ((int) j + 2) + 10) / N;
		B[state] = ((num_t) ((int) i - 4) * ((int)j - 1) + 11) / N;
	});
}

template<class inner_t, class A_t, class B_t>
__global__ void jacobi2d_kernel1(inner_t inner, A_t A, B_t B) {
	// A: i x j
	// B: i x j

	inner.template for_dims<'s', 't'>([=](auto inner){
		inner.for_each([=](auto state) {
			B[state] = (num_t).2 * (
				A[state] +
				A[neighbor<'j'>(state, -1)] +
				A[neighbor<'j'>(state, +1)] +
				A[neighbor<'i'>(state, +1)] +
				A[neighbor<'i'>(state, -1)]);
		});
	});
}

template<class inner_t, class A_t, class B_t>
__global__ void jacobi2d_kernel2(inner_t inner, A_t A, B_t B) {
	// A: i x j
	// B: i x j

	inner.template for_dims<'s', 't'>([=](auto inner){
		inner.for_each([=](auto state) {
			A[state] = B[state];
		});
	});
}

// run kernels
void run_jacobi2d(std::size_t tsteps, auto A, auto B) {
	// A: i x j
	// B: i x j

	auto trav = noarr::traverser(A, B)
		.order(noarr::symmetric_spans<'i', 'j'>(A, 1, 1))
		.order(noarr::bcast<'T'>(tsteps))
		.order(noarr::into_blocks_dynamic<'i', 'I', 'i', 's'>(DIM_THREAD_BLOCK_Y))
		.order(noarr::into_blocks_dynamic<'j', 'J', 'j', 't'>(DIM_THREAD_BLOCK_X));

	trav.template for_dims<'T'>([A, B](auto inner) {
		noarr::cuda_threads<'J', 'j', 'I', 'i'>(inner)
			.simple_run(jacobi2d_kernel1, 0, A, B);

		CUCH(cudaGetLastError());
		CUCH(cudaDeviceSynchronize());

		noarr::cuda_threads<'J', 'j', 'I', 'i'>(inner)
			.simple_run(jacobi2d_kernel2, 0, A, B);

		CUCH(cudaGetLastError());
		CUCH(cudaDeviceSynchronize());
	});
}

class experiment : public virtual_experiment {
	template<class A, class B>
	struct experiment_data : public virtual_data {
		std::size_t tsteps;
		A a;
		B b;

		experiment_data(std::size_t tsteps, A a, B b)
			: tsteps(tsteps), a(std::move(a)), b(std::move(b)) { }

		void run() override {
			run_jacobi2d(tsteps, a.get_device_ref(), b.get_device_ref());
		}

		void print_results(std::ostream& os) override {
			a.fetch_to_host();
			noarr::serialize_data(os, a.get_host_ref() ^ noarr::hoist<'i'>());
		}
	};

public:
	experiment() {
		// problem size
		std::size_t tsteps = TSTEPS;
		std::size_t n = N;

		cudaInit();

		// data
		experiment_data new_data{
			tsteps,
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'i'>(n, n)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'i'>(n, n))
		};

		init(new_data.a.get_host_ref(), new_data.b.get_host_ref());

		new_data.a.fetch_to_device();
		new_data.b.fetch_to_device();

		data = std::make_unique<decltype(new_data)>(std::move(new_data));
	}
};


} // namespace

REGISTER_EXPERIMENT(jacobi2d);
