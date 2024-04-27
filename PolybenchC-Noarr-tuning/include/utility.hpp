#ifndef NOARR_POLYBENCH_TUNING_UTILITY_HPP
#define NOARR_POLYBENCH_TUNING_UTILITY_HPP

#include <utility>


#include <noarr/traversers.hpp>

template<noarr::IsDim auto Dim, noarr::IsDim auto Major, noarr::IsDim auto Minor, noarr::IsDim auto Guard>
struct into_pow2_blocks_t {
    constexpr auto operator()(auto block_size) const noexcept {
        if constexpr (block_size == 0)
            return noarr::bcast<Major>(noarr::lit<1>);
        else
            return noarr::into_blocks_dynamic<Dim, Major, Minor, Guard>(noarr::lit<1 << block_size>);
    }
};

/**
    * @brief A function object that returns a noarr::into_blocks_dynamic with the block size set to 2^block_size
*/
template<noarr::IsDim auto Dim, noarr::IsDim auto Major, noarr::IsDim auto Minor, noarr::IsDim auto Guard>
constexpr into_pow2_blocks_t<Dim, Major, Minor, Guard> into_pow2_blocks;

struct xor_fold_t {
    constexpr decltype(auto) operator()(auto &&...items) const {
        return (... ^ std::forward<decltype(items)>(items));
    }
};

/**
    * @brief A function object that returns the XOR fold of the given values
*/
constexpr xor_fold_t xor_fold;

#endif // NOARR_POLYBENCH_TUNING_UTILITY_HPP
