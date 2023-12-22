#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

#include "common.hpp"

int main(int argc, char** argv) {
	using namespace std::string_literals;

	// parse arguments
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <experiment> [args...]" << std::endl;

		virtual_experiment::print_experiments(std::cerr);

		return 1;
	}

	// create experiment
	auto experiment = virtual_experiment::get_experiment(argv[1]);

	auto start = std::chrono::high_resolution_clock::now();

	// run kernels
	experiment->run();

	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration<double>(end - start);

	std::cerr << duration.count() << std::endl;

	// print results
	if (argv[0] != ""s) {
        std::cout << std::fixed << std::setprecision(2);
		experiment->print_results(std::cout);
	}
}
