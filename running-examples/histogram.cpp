#define CPU
#include "histomain.hpp"

#include <noarr/traversers.hpp>

#ifndef HISTO_IMPL
#error Add -DHISTO_IMPL=(histo_loop|histo_range|histo_foreach|histo_tbbreduce) to compiler commandline
#define HISTO_IMPL histo_undefined
#endif

#ifdef HISTO_HAVE_TBB
#include <noarr/structures/interop/tbb.hpp>
#endif

enum {
	histo_loop,
	histo_range,
	histo_foreach,
	histo_tbbreduce,
	histo_undefined
};

void run_histogram(value_t *in_ptr, std::size_t size, std::size_t *out_ptr) {

if constexpr (HISTO_IMPL == histo_loop) {
	auto in = noarr::bag(noarr::scalar<value_t>() ^ noarr::vector<'i'>(size), in_ptr);
	auto out = noarr::bag(noarr::scalar<std::size_t>() ^ noarr::array<'v', 256>(), out_ptr);

	for(std::size_t i = 0; i < size; ++i) {
		out[noarr::idx<'v'>(in[noarr::idx<'i'>(i)])] += 1;
	}
}

else if constexpr (HISTO_IMPL == histo_range) {
	auto in = noarr::bag(noarr::scalar<value_t>() ^ noarr::vector<'i'>(size), in_ptr);
	auto out = noarr::bag(noarr::scalar<std::size_t>() ^ noarr::array<'v', 256>(), out_ptr);

	for(auto elem : noarr::traverser(in)) {
		out[noarr::idx<'v'>(in[elem.state()])] += 1;
	}
}

else if constexpr (HISTO_IMPL == histo_foreach) {
	auto in = noarr::bag(noarr::scalar<value_t>() ^ noarr::vector<'i'>(size), in_ptr);
	auto out = noarr::bag(noarr::scalar<std::size_t>() ^ noarr::array<'v', 256>(), out_ptr);

	// PAPER 4.0 - First listing
	noarr::traverser(in) | [=](auto state) {
		out[noarr::idx<'v'>(in[state])] += 1;
	};
}

#ifdef HISTO_HAVE_TBB
else if constexpr (HISTO_IMPL == histo_tbbreduce) {
	auto in = noarr::bag(noarr::scalar<value_t>() ^ noarr::vector<'i'>(size), in_ptr);
	auto out = noarr::bag(noarr::scalar<std::size_t>() ^ noarr::array<'v', 256>(), out_ptr);

	// PAPER 4.0 - Second listing
	noarr::tbb_reduce(
		// Input traverser.
		noarr::traverser(in),

		// Neutralizing function, OutElem := 0
		[](auto out_state, auto &out_left) {
			out_left[out_state] = 0;
		},

		// Accumulation function, Out += InElem
		[in](auto in_state, auto &out_left) {
			out_left[noarr::idx<'v'>(in[in_state])] += 1;
		},

		// Joining function, OutElem += OutElem
		[](auto out_state, auto &out_left, const auto &out_right) {
			out_left[out_state] += out_right[out_state];
		},

		// Output bag.
		out
	);
}
#endif

}
