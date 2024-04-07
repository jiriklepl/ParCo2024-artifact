#include <memory>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>
#include <noarr/structures/interop/cuda_traverser.cuh>

#include "common.hpp"
#include "defines.cuh"
#include "2mm.cuh"

using num_t = DATA_TYPE;

namespace {

// initialize data
void init(num_t &alpha, num_t &beta, auto A, auto B, auto C, auto D) {
	// A: ni x nk
	// B: nk x nj
	// C: nl x nj
	// D: ni x nl
	using namespace noarr;

	alpha = 32412;
	beta = 2123;

	traverser(A) | [=](auto state) {
		auto [i, k] = get_indices<'i', 'k'>(state);
		A[state] = ((num_t) i * k) / NI;
	};

	traverser(B) | [=](auto state) {
		auto [k, j] = get_indices<'k', 'j'>(state);
		B[state] = ((num_t) k * (j + 1)) / NJ;
	};

	traverser(C) | [=](auto state) {
		auto [l, j] = get_indices<'l', 'j'>(state);
		C[state] = ((num_t) l * (j + 3)) / NL;
	};

	traverser(D) | [=](auto state) {
		auto [i, l] = get_indices<'i', 'l'>(state);
		D[state] = ((num_t) i * (l + 2)) / NK;
	};
}

template<class inner_t, class tmp_t, class A_t, class B_t>
__global__ void kernel_2mm_1(inner_t inner, num_t alpha, [[maybe_unused]] num_t beta, tmp_t tmp, A_t A, B_t B) {
	using namespace noarr;

	inner | for_dims<'s', 't'>([=](auto inner) {
		tmp[inner] = 0;

		inner | [=](auto state) {
			tmp[state] += alpha * A[state] * B[state];
		};
	});
}

template<class inner_t, class tmp_t, class C_t, class D_t>
__global__ void kernel_2mm_2(inner_t inner, [[maybe_unused]] num_t alpha, num_t beta, tmp_t tmp, C_t C, D_t D) {
	using namespace noarr;

	inner | for_dims<'s', 'v'>([=](auto inner) {
		D[inner] *= beta;

		inner | [=](auto state) {
			D[state] += tmp[state] * C[state];
		};
	});
}

// run kernels
void run_2mm(num_t alpha, num_t beta, auto tmp, auto A, auto B, auto C, auto D) {
	// tmp: ni x nj
	// A: ni x nk
	// B: nk x nj
	// C: nl x nj
	// D: ni x nl

	auto trav1 = noarr::traverser(tmp, A, B) ^
		noarr::into_blocks_dynamic<'i', 'I', 'i', 's'>(DIM_THREAD_BLOCK_Y) ^
		noarr::into_blocks_dynamic<'j', 'J', 'j', 't'>(DIM_THREAD_BLOCK_X);
	
	auto trav2 = noarr::traverser(tmp, C, D) ^
		noarr::into_blocks_dynamic<'i', 'I', 'i', 's'>(DIM_THREAD_BLOCK_Y) ^
		noarr::into_blocks_dynamic<'l', 'L', 'l', 'v'>(DIM_THREAD_BLOCK_X);

	noarr::cuda_threads<'J', 'j', 'I', 'i'>(trav1)
		.simple_run(kernel_2mm_1, 0, alpha, beta, tmp, A, B);

	CUCH(cudaGetLastError()); // check for configuration errors
	CUCH(cudaDeviceSynchronize()); // join, check for execution errors
	
	noarr::cuda_threads<'L', 'l', 'I', 'i'>(trav2)
		.simple_run(kernel_2mm_2, 0, alpha, beta, tmp, C, D);

	CUCH(cudaGetLastError()); // check for configuration errors
	CUCH(cudaDeviceSynchronize()); // join, check for execution errors
}

class experiment : public virtual_experiment {
	template<class TMP, class A, class B, class C, class D>
	struct experiment_data : public virtual_data {
		TMP tmp;
		A a;
		B b;
		C c;
		D d;
		num_t alpha = 0;
		num_t beta = 0;

		experiment_data(TMP tmp, A a, B b, C c, D d)
			: tmp(std::move(tmp)), a(std::move(a)), b(std::move(b)), c(std::move(c)), d(std::move(d)) { }

		void run() override {
			run_2mm(alpha, beta, tmp.get_device_ref(), a.get_device_ref(), b.get_device_ref(), c.get_device_ref(), d.get_device_ref());
		}

		void print_results(std::ostream& os) override {
			d.fetch_to_host();
			noarr::serialize_data(os, d.get_host_ref() ^ noarr::hoist<'i'>());
		}
	};

public:
	experiment() {
		// problem size
		std::size_t ni = NI;
		std::size_t nj = NJ;
		std::size_t nk = NK;
		std::size_t nl = NL;

		cudaInit();

		// data
		experiment_data new_data{
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'i'>(nj, ni)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'k', 'i'>(nk, ni)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'k'>(nj, nk)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'l'>(nj, nl)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'l', 'i'>(nl, ni))
		};

		init(new_data.alpha, new_data.beta, new_data.a.get_host_ref(), new_data.b.get_host_ref(), new_data.c.get_host_ref(), new_data.d.get_host_ref());

		new_data.a.fetch_to_device();
		new_data.b.fetch_to_device();
		new_data.c.fetch_to_device();
		new_data.d.fetch_to_device();

		data = std::make_unique<decltype(new_data)>(std::move(new_data));
	}
};


} // namespace

REGISTER_EXPERIMENT(2mm);
