#ifndef _JACOBI2D_CUH
#define _JACOBI2D_CUH

#include "defines.cuh"

#ifdef MINI_DATASET
# define TSTEPS 20
# define N 1000
#elif defined(SMALL_DATASET)
# define TSTEPS 20
# define N 1000
#elif defined(MEDIUM_DATASET)
# define TSTEPS 100
# define N 10000
#elif defined(LARGE_DATASET)
# define TSTEPS 20
# define N 1000
#elif defined(EXTRALARGE_DATASET)
# define TSTEPS 20
# define N 1000
#endif

/* Thread block dimensions */
#define DIM_THREAD_BLOCK_X 32
#define DIM_THREAD_BLOCK_Y 8

#endif // _JACOBI2D_CUH
