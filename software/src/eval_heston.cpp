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
#include "heston_both_cpu.hpp"
#include "json_helper.hpp"

#include "json/json.h"

#include <iostream>
#include <cmath>
#include <chrono>


void print_usage_and_exit(char *argv0) {
	std::cerr << "Usage: " << argv0 << "-<algorithm> params.json" << std::endl;
	std::cerr << "    -<algorithm>   : sl, ml or both" << std::endl;
#ifdef WITH_MPI
	MPI_Finalize();
#endif
	exit(1);
}

void check_usage(int argc, char *argv[], bool &eval_sl, bool &eval_ml) {
	if (argc != 3)
		print_usage_and_exit(argv[0]);

	// algorithm
	std::string algorithm = argv[1];
	if (algorithm == "-sl") {
		eval_sl = true;
		eval_ml = false;
	} else if (algorithm == "-ml") {
		eval_sl = false;
		eval_ml = true;
	} else if (algorithm == "-both") {
		eval_sl = eval_ml = true;
	} else
		print_usage_and_exit(argv[0]);
}


void eval_heston(const HestonParamsEval &eval_params, 
		const bool eval_multilevel, const bool do_print) {
	if (do_print)
		if (eval_multilevel)
			std::cout << "  \"multi-level\": [" << std::endl;
		else
			std::cout << "  \"single-level\": [" << std::endl;
	bool first = true;
	for (int level = eval_params.eval_start_level; 
			level <= eval_params.eval_stop_level; ++level) {
		uint32_t step_cnt = std::pow(eval_params.ml_constant, 
				level + eval_params.ml_start_level);
		bool do_multilevel = (level != 0) && eval_multilevel;

		Statistics stats;
		auto start = std::chrono::steady_clock::now();
		stats = heston_cpu_kernel<float>(eval_params, step_cnt, eval_params.path_cnt,
				do_multilevel, eval_params.ml_constant);
		auto end = std::chrono::steady_clock::now();
		double duration = std::chrono::duration<double>(
				end - start).count();
		double sigma = std::sqrt(stats.variance / stats.cnt);
		// https://en.wikipedia.org/w/index.php?title=Normal_distribution&oldid=563679165#Estimation_of_parameters
		double var_sigma = stats.variance * std::sqrt(2. / stats.cnt);
		if (do_print) {
			if (!first) {
				std::cout << ", " << std::endl;
			}
			std::cout << "    {\"step_cnt\": " << step_cnt << 
					", \"stats\": " << stats << ", \"duration\": " <<
					duration << ", \"sigma\": " << sigma << ", \"var_sigma\": " <<
					var_sigma << "}" << std::flush;
		}
		first = false;
	}
	if (do_print)
		std::cout << std::endl << "  ]";
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
	bool do_eval_sl, do_eval_ml;
	check_usage(argc, argv, do_eval_sl, do_eval_ml);
	Json::Value json = read_params(argv[2]);
	HestonParamsEval eval_params = get_eval_params(json);

	if (do_print)
		std::cout << "{" << std::endl;
	if (do_eval_ml)
		eval_heston(eval_params, true, do_print);
	if (do_print && do_eval_ml && do_eval_sl && do_print)
		std::cout << "," << std::endl;
	if (do_eval_sl)
		eval_heston(eval_params, false, do_print);
	if (do_print)
		std::cout << std::endl << "}" << std::endl;

#ifdef WITH_MPI
	MPI_Finalize();
#endif
	return 0;
}


