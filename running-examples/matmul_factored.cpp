#define CPU
#include "matmulmain.hpp"

template<class A, class B, class C>
void run_matmul(A ta, B tb, C tc, num_t *pa, num_t *pb, num_t *pc) {
	auto a = noarr::bag(ta, pa);
	auto b = noarr::bag(tb, pb);
	auto c = noarr::bag(tc, pc);

	noarr::traverser(c) | [=](auto state) { c[state] = 0; };

	auto tiles = noarr::into_blocks<'i', 'I'>(noarr::lit<16>) ^
		noarr::into_blocks<'k', 'K'>(noarr::lit<16>) ^
		noarr::into_blocks<'j', 'J'>(noarr::lit<16>) ^
		noarr::hoist<'I', 'J', 'K'>();

	// PAPER: 3.2 - First listing
	noarr::traverser(a, b, c) ^ tiles | noarr::for_dims<'I', 'J', 'K', 'j', 'i'>([=](auto inner_trav) {
		auto res = c[inner_trav.state()]; // local variable (register)

		inner_trav | [=, &res](auto state){
			res += a[state] * b[state];
		};

		c[inner_trav.state()] = res;
	});
}
