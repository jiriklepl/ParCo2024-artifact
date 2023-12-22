#ifndef _GRAMSCHM_CUH
#define _GRAMSCHM_CUH

#include "defines.cuh"

#ifdef MINI_DATASET
# define NI 2048
# define NJ 2048
#elif defined(SMALL_DATASET)
# define NI 2048
# define NJ 2048
#elif defined(MEDIUM_DATASET)
# define NI 2000
# define NJ 2300
#elif defined(LARGE_DATASET)
# define NI 2048
# define NJ 2048
#elif defined(EXTRALARGE_DATASET)
# define NI 2048
# define NJ 2048
#endif

/* Thread block dimensions */
#define DIM_THREAD_BLOCK_X 256
#define DIM_THREAD_BLOCK_Y 1

#endif // _GRAMSCHM_CUH
