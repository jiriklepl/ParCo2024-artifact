#ifndef _2DCONVOLUTION_CUH
#define _2DCONVOLUTION_CUH

#include "defines.cuh"

#ifdef MINI_DATASET
# define NI 4096
# define NJ 4096
#elif defined(SMALL_DATASET)
# define NI 4096
# define NJ 4096
#elif defined(MEDIUM_DATASET)
# define NI 40000
# define NJ 43000
#elif defined(LARGE_DATASET)
# define NI 4096
# define NJ 4096
#elif defined(EXTRALARGE_DATASET)
# define NI 4096
# define NJ 4096
#endif

/* Thread block dimensions */
#define DIM_THREAD_BLOCK_X 32
#define DIM_THREAD_BLOCK_Y 8

#endif // _2DCONVOLUTION_CUH
