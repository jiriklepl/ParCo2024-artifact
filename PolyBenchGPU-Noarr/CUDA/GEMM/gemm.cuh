#ifndef _GEMM_CUH
#define _GEMM_CUH

#include "defines.cuh"

#ifdef MINI_DATASET
# define NI 512
# define NJ 512
# define NK 512
#elif defined(SMALL_DATASET)
# define NI 512
# define NJ 512
# define NK 512
#elif defined(MEDIUM_DATASET)
# define NI 5000
# define NJ 5300
# define NK 5600
#elif defined(LARGE_DATASET)
# define NI 512
# define NJ 512
# define NK 512
#elif defined(EXTRALARGE_DATASET)
# define NI 512
# define NJ 512
# define NK 512
#endif

/* Thread block dimensions */
#define DIM_THREAD_BLOCK_X 32
#define DIM_THREAD_BLOCK_Y 8

#endif // _GEMM_CUH
