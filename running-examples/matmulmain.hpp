#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <sstream>

#include <noarr/traversers.hpp>

#ifdef CUDA
#include <noarr/structures/interop/cuda_traverser.cuh>

#define CUCH(status)  do { cudaError_t err = status; if (err != cudaSuccess) std::cerr << __FILE__ ":" << __LINE__ << ": error: " << cudaGetErrorString(err) << "\n\t" #status << std::endl, exit(err); } while (false)
#endif

using num_t = float;

template<class A, class B, class C>
extern void run_matmul(A ta, B tb, C tc, num_t *pa, num_t *pb, num_t *pc);

int main(int argc, char **argv) {
#ifdef MATRIX_SIZE
	if(argc != 2) {
		std::cerr << "Usage: PROGRAM FILE" << std::endl;
		return 1;
	}

	auto i_st = noarr::array<'i', (std::size_t)MATRIX_SIZE>();
	auto j_st = noarr::array<'j', (std::size_t)MATRIX_SIZE>();
	auto k_st = noarr::array<'k', (std::size_t)MATRIX_SIZE>();
#else
	if(argc != 3) {
		std::cerr << "Usage: PROGRAM FILE SIZE" << std::endl;
		return 1;
	}

	std::size_t size;

	{
		std::istringstream size_stream(argv[2]);
		size_stream >> size;
	}
#endif

	auto i_st = noarr::sized_vector<'i'>(size);
	auto j_st = noarr::sized_vector<'j'>(size);
	auto k_st = noarr::sized_vector<'k'>(size);

	auto ta = noarr::scalar<num_t>() ^ i_st ^ k_st;
	auto tb = noarr::scalar<num_t>() ^ k_st ^ j_st;
	auto tc = noarr::scalar<num_t>() ^ i_st ^ j_st;

	std::size_t a_sz = ta | noarr::get_size();
	std::size_t b_sz = tb | noarr::get_size();
	std::size_t c_sz = tc | noarr::get_size();

	num_t *data;

#ifdef CUDA
	CUCH(cudaMallocManaged(&data, a_sz + b_sz + c_sz));
#else
	if (!(data = (num_t *)malloc(a_sz + b_sz + c_sz))) {
		std::cerr << __FILE__ ":" << __LINE__ << ": error: failed to allocate memory" << std::endl;
		exit(1);
	}
#endif

	std::FILE *file = std::fopen(argv[1], "r");
	if(std::fread(data, 1, a_sz + b_sz, file) != a_sz + b_sz) {
		std::cerr << "Input error" << std::endl;
		return 1;
	}
	std::fclose(file);

	auto start = std::chrono::high_resolution_clock::now();
	run_matmul(ta, tb, tc, data, (data + a_sz / sizeof(num_t)), (data + (a_sz + b_sz) / sizeof(num_t)));
	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<long double>(end - start);
	std::cerr << duration.count() << std::endl;

	std::fwrite(data + (a_sz + b_sz) / sizeof(num_t), 1, c_sz, stdout);

#ifdef CUDA
	CUCH(cudaFree(data));
#else
	free(data);
#endif

	return 0;
}
