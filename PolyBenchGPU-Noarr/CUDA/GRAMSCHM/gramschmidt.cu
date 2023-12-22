#include <memory>

#include <noarr/structures_extended.hpp>
#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures/interop/bag.hpp>
#include <noarr/structures/interop/serialize_data.hpp>
#include <noarr/structures/interop/cuda_traverser.cuh>

#include "common.hpp"
#include "defines.cuh"
#include "gramschmidt.cuh"
#include "noarr/structures/extra/shortcuts.hpp"

using num_t = DATA_TYPE;

namespace {

// initialize data
void init(auto A, auto R, auto Q) {
	// A: i x k
	// R: k x j
	// Q: i x k

	auto ni = A | noarr::get_length<'i'>();
	auto nj = R | noarr::get_length<'j'>();

	noarr::traverser(A, Q).for_each([=](auto state) {
		auto i = noarr::get_index<'i'>(state);
		auto k = noarr::get_index<'k'>(state);

		A[state] = ((num_t) i * k) / ni;
		Q[state] = ((num_t) i * (k + 1)) / nj;
	});

	noarr::traverser(R).for_each([=](auto state) {
		auto [k, j] = noarr::get_indices<'k', 'j'>(state);

		R[state] = ((num_t) k * (j + 2)) / nj;
	});
}

template<class inner_t, class A_t, class R_t, class Q_t>
__global__ void gramschmidt_kernel1(inner_t inner, A_t A, R_t R_diag, [[maybe_unused]] Q_t Q) {
	// A: i x k
	// R: k x j
	// Q: i x k

	inner.template for_dims<'t'>([=](auto inner) {
		auto state = inner.state();

		num_t nrm = 0;

		inner.template for_each<'i'>([=, &nrm](auto state) {
			nrm += A[state] * A[state];
		});

		R_diag[state] = sqrt(nrm);
	});
}

template<class inner_t, class A_t, class R_t, class Q_t>
__global__ void gramschmidt_kernel2(inner_t inner, A_t A, R_t R_diag, Q_t Q) {
	// A: i x k
	// R: k x j
	// Q: i x k

	inner.template for_each<'s'>([=](auto state) {
		Q[state] = A[state] / R_diag[state];
	});
}

template<class inner_t, class A_t, class R_t, class Q_t>
__global__ void gramschmidt_kernel3(inner_t inner, A_t A_ij, R_t R, Q_t Q) {
	// A: i x k
	// R: k x j
	// Q: i x k

	inner.template for_dims<'t'>([=](auto inner) {
		auto state = inner.state();
		auto [j, k] = noarr::get_indices<'j', 'k'>(state);

		if (j <= k)
			return;


		R[state] = 0;

		inner.template for_each<'i'>([=](auto state) {
			R[state] += Q[state] * A_ij[state];
		});

		inner.template for_each<'i'>([=](auto state) {
			A_ij[state] -= Q[state] * R[state];
		});
	});
}

// run kernels
void run_gramschmidt(auto A, auto R, auto Q) {
	// A: i x k
	// R: k x j
	// Q: i x k

	auto trav = noarr::traverser(A, R, Q);

	// `A_ij = A ^ noarr::rename<'k', 'j'>()` currently triggers a compiler bug, this is a simple workaround
	auto A_ij = noarr::make_bag(noarr::scalar<num_t>() ^ noarr::vectors_like<'j', 'i'>(trav.top_struct()), A.data());

	trav.template for_dims<'k'>([=](auto inner) {
		auto trav1 = inner
			.order(noarr::slice<'j'>(0, 1))
			.order(noarr::into_blocks_dynamic<'j', 'J', 'j', 't'>(DIM_THREAD_BLOCK_X))
			.order(noarr::bcast<'Y'>(1) ^ noarr::bcast<'y'>(DIM_THREAD_BLOCK_Y))
			;
	
		auto trav2 = inner
			.order(noarr::into_blocks_dynamic<'i', 'I', 'i', 's'>(DIM_THREAD_BLOCK_X))
			.order(noarr::bcast<'Y'>(1) ^ noarr::bcast<'y'>(DIM_THREAD_BLOCK_Y))
			;

		auto trav3 = inner
			.order(noarr::into_blocks_dynamic<'j', 'J', 'j', 't'>(DIM_THREAD_BLOCK_X))
			.order(noarr::bcast<'Y'>(1) ^ noarr::bcast<'y'>(DIM_THREAD_BLOCK_Y))
			;

		auto R_diag = R ^ noarr::fix<'j'>(noarr::get_index<'k'>(inner.state()));

		noarr::cuda_threads<'J', 'j', 'Y', 'y'>(trav1)
			.simple_run(gramschmidt_kernel1, 0, A, R_diag, Q);

		CUCH(cudaGetLastError());
		CUCH(cudaDeviceSynchronize());

		noarr::cuda_threads<'I', 'i', 'Y', 'y'>(trav2)
			.simple_run(gramschmidt_kernel2, 0, A, R_diag, Q);
		
		CUCH(cudaGetLastError());
		CUCH(cudaDeviceSynchronize());

		noarr::cuda_threads<'J', 'j', 'Y', 'y'>(trav3)
			.simple_run(gramschmidt_kernel3, 0, A_ij, R, Q);
		
		CUCH(cudaGetLastError());
		CUCH(cudaDeviceSynchronize());
	});
}

class experiment : public virtual_experiment {
	template<class A, class R, class Q>
	struct experiment_data : public virtual_data {
		A a;
		R r;
		Q q;

		experiment_data(A a, R r, Q q)
			: a(std::move(a)), r(std::move(r)), q(std::move(q)) { }

		void run() override {
			run_gramschmidt(a.get_device_ref(), r.get_device_ref(), q.get_device_ref());
		}

		void print_results(std::ostream& os) override {
			a.fetch_to_host();
			noarr::serialize_data(os, a.get_host_ref() ^ noarr::hoist<'i'>());
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
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'k', 'i'>(nj, ni)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'j', 'k'>(nj, nj)),
			managed_bag(noarr::scalar<num_t>() ^ noarr::sized_vectors<'k', 'i'>(nj, ni))
		};

		init(new_data.a.get_host_ref(), new_data.r.get_host_ref(), new_data.q.get_host_ref());

		new_data.a.fetch_to_device();
		new_data.r.fetch_to_device();
		new_data.q.fetch_to_device();

		data = std::make_unique<decltype(new_data)>(std::move(new_data));
	}
};


} // namespace

REGISTER_EXPERIMENT(gramschmidt);
