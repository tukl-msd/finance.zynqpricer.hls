//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 29. September 2013
//

#ifdef WITH_MPI
	#include "mpi.h"
#endif

#include "heston_sl_cpu.hpp"
#include "json_helper.hpp"

#include "json/json.h"

#include <iostream>
#include <cmath>
#include <chrono>


void print_usage_and_exit(char *argv0) {
	std::cerr << "Usage: " << argv0 << "params.json" << std::endl;
#ifdef WITH_MPI
	MPI_Finalize();
#endif
	exit(1);
}


int main(int argc, char *argv[]) {
	bool do_print = true;
#ifdef WITH_MPI
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	do_print = rank == 0;
#endif

	// read parameters
	if (argc != 2) {
		print_usage_and_exit(argv[0]);
	}
	Json::Value json = read_params(argv[1]);
	HestonParamsSL sl_params = get_sl_params(json);

	for (int level = 0; level <= 16; ++level) {
		sl_params.step_cnt = std::pow(2, level);

		Statistics stats;
		auto start = std::chrono::steady_clock::now();
		stats = heston_cpu_kernel_sl<float>(sl_params, sl_params.step_cnt, 
				sl_params.path_cnt);
		auto end = std::chrono::steady_clock::now();
		double duration = std::chrono::duration<double>(
				end - start).count();
		double sigma = std::sqrt(stats.variance / stats.cnt);
		if (do_print)
			std::cout << "{step_cnt: " << sl_params.step_cnt << 
					", stats: " << stats << ", duration: " <<
					duration << ", sigma: " << sigma << "}" << 
					std::endl << std::flush;
	}

#ifdef WITH_MPI
	MPI_Finalize();
#endif
	return 0;
}


