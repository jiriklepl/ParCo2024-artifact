#ifndef _2MM_CUH
#define _2MM_CUH

#include "defines.cuh"

#ifdef MINI_DATASET
# define NI 2048
# define NJ 2048
# define NK 2048
# define NL 2048
#elif defined(SMALL_DATASET)
# define NI 2048
# define NJ 2048
# define NK 2048
# define NL 2048
#elif defined(MEDIUM_DATASET)
# define NI 5000
# define NJ 5200
# define NK 5300
# define NL 5600
#elif defined(LARGE_DATASET)
# define NI 2048
# define NJ 2048
# define NK 2048
# define NL 2048
#elif defined(EXTRALARGE_DATASET)
# define NI 2048
# define NJ 2048
# define NK 2048
# define NL 2048
#endif

#define DIM_THREAD_BLOCK_X 32
#define DIM_THREAD_BLOCK_Y 8

#endif // _2MM_CUH
