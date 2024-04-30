#define CPU
#include "matmulmain.hpp"

template<class A, class B, class C>
void run_matmul(A ta, B tb, C tc, num_t *pa, num_t *pb, num_t *pc) {
	auto a = noarr::bag(ta, pa);
	auto b = noarr::bag(tb, pb);
	auto c = noarr::bag(tc, pc);

	// PAPER: 3.1 - First listing
	noarr::traverser(c) | [=](auto state) { c[state] = 0; };

#ifdef MATMUL_BLOCKED
	// PAPER: 3.1 - Third listing
	auto blocks = noarr::strip_mine<'i', 'I', 'i'>(noarr::lit<16>) ^
		noarr::strip_mine<'k', 'K', 'k'>(noarr::lit<16>) ^
		noarr::strip_mine<'j', 'J', 'j'>(noarr::lit<16>);

	noarr::traverser(a, b, c) ^ blocks | [=](auto state) {
		c[state] += a[state] * b[state];
	};
#else
	// PAPER: 3.1 - Second listing
	noarr::traverser(a, b, c) | [=](auto state) {
		c[state] += a[state] * b[state];
	};
#endif
}
