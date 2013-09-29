//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#ifdef WITH_MPI
	#include "mpi.h"
#endif

#include "heston_sl_cpu.hpp"
#ifdef WITH_ACC
	#include "heston_sl_acc.hpp"
	#include "heston_ml_acc.hpp"
#endif
#include "json_helper.hpp"

#include "json/json.h"

#include <chrono>
#include <iostream>
#include <string>


void print_duration(std::chrono::steady_clock::time_point start, 
		std::chrono::steady_clock::time_point end, uint64_t steps) {
	double duration = std::chrono::duration<double>(
		end - start).count();
	std::cout << "Calculated " << steps << " steps in " << duration
		<< " seconds (" << steps / duration << " steps / sec)"
		<< std::endl;
}


void print_usage_and_exit(char *argv0) {
	std::cerr << "Usage: " << argv0 << " -<algorithm> -<architecture> " <<
			"params.json" << " [bitstream.json]" << std::endl;
	std::cerr << "    -<algorithm>   : sl, ml or both" << std::endl;
	std::cerr << "    -<architecture>: cpu, acc or both" << std::endl;
#ifdef WITH_MPI
	MPI_Finalize();
#endif
	exit(1);
}


void check_usage(int argc, char *argv[], bool &run_sl, bool &run_ml, 
		bool &run_acc, bool &run_cpu) {
	if (argc < 3)
		print_usage_and_exit(argv[0]);

	// algorithm
	std::string algorithm = argv[1];
	if (algorithm == "-sl") {
		run_sl = true;
		run_ml = false;
	} else if (algorithm == "-ml") {
		run_sl = false;
		run_ml = true;
	} else if (algorithm == "-both") {
		run_sl = run_ml = true;
	} else
		print_usage_and_exit(argv[0]);

	// architecture
	std::string architecture = argv[2];
	if (architecture == "-cpu") {
		run_cpu = true;
		run_acc = false;
	} else if (architecture == "-acc") {
		run_cpu = false;
		run_acc = true;
	} else if (architecture == "-both") {
		run_cpu = run_acc = true;
	} else
		print_usage_and_exit(argv[0]);

	// check bitfile json
	if ((run_acc && argc != 5) || (!run_acc && argc != 4)) {
		print_usage_and_exit(argv[0]);
	}
}


int main(int argc, char *argv[]) {
#ifdef WITH_MPI
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif
	bool run_sl, run_ml, run_acc, run_cpu;
	check_usage(argc, argv, run_sl, run_ml, run_acc, run_cpu);
	// read json files
	Json::Value json = read_params(argv[3]);
	Json::Value bitstream;
	if (run_acc)
		bitstream = read_params(argv[4]);

	// heston params
	HestonParamsSL sl_params = get_sl_params(json);
	HestonParamsML ml_params = get_ml_params(json);
	// reference values
	auto ref = json["reference"];
	double ref_price = ref["price"].asDouble();
	double ref_price_precision = ref["precision"].asDouble();

#ifdef WITH_MPI
	if (rank == 0) {
#endif
	// benchmark
	std::cout << "REF   : result = " << ref_price << std::endl;
	uint64_t steps = sl_params.step_cnt * sl_params.path_cnt;
#ifdef WITH_ACC
	if (run_acc && run_sl) {
		auto start_acc = std::chrono::steady_clock::now();
		double result_acc = heston_sl_hw(bitstream, sl_params);
		auto end_acc = std::chrono::steady_clock::now();
		std::cout << "ACC-SL: result = " << result_acc << std::endl;
		std::cout << "ACC-SL: "; print_duration(start_acc, end_acc, steps);
	}
	if (run_acc && run_ml) {
		auto start_acc = std::chrono::steady_clock::now();
		double result_acc = heston_ml_hw(bitstream, ml_params);
		auto end_acc = std::chrono::steady_clock::now();
		std::cout << "ACC-ML: result = " << result_acc << std::endl;
		std::cout << "ACC-ML: "; print_duration(start_acc, end_acc, 0);
	}
#else
	if (run_acc) {
		std::cerr << "ERROR: recompile with accelerator support" << 
				std::endl;
		return -1;
	}
#endif
	if (run_cpu && run_sl) {
		auto start_cpu = std::chrono::steady_clock::now();
		double result_cpu = heston_sl_cpu<float>(sl_params);
		auto end_cpu = std::chrono::steady_clock::now();
		std::cout << "CPU-SL: result = " << result_cpu << std::endl;
		std::cout << "CPU-SL: "; print_duration(start_cpu, end_cpu, steps);
	}
	if (run_cpu && run_ml) {
		std::cout << "CPU-ML: not implemented yet" << std::endl;
	}

#ifdef WITH_MPI
	} else {
		if (run_cpu && run_sl) {
			heston_sl_cpu<float>(sl_params);
		}
	}
	MPI_Finalize();
#endif
	return 0;
}


