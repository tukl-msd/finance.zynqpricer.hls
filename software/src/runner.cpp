//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#include "runner.hpp"

#include "heston_sl_cpu.hpp"
#ifdef __unix__
#include "heston_sl_acc.hpp"
#endif
#include "json_helper.hpp"

#include "json/json.h"

#include <chrono>
#include <iostream>


void print_duration(std::chrono::steady_clock::time_point start, 
		std::chrono::steady_clock::time_point end, uint64_t steps) {
	double duration = std::chrono::duration<double>(
		end - start).count();
	std::cout << "Calculated " << steps << " steps in " << duration
		<< " seconds (" << steps / duration << " steps / sec)"
		<< std::endl;
}


void check_usage(int argc, char *argv0, bool run_acc) {
	if ((run_acc && argc != 3) || (!run_acc && argc != 2)) {
		std::cerr << "Usage: " << argv0 << " params.json" << 
				(run_acc ? " bitstream.json" : "") << std::endl;
		exit(-1);
	}
}


int main_runner(int argc, char *argv[], bool run_cpu, bool run_acc)
{
	check_usage(argc, argv[0], run_acc);

	Json::Value json = read_params(argv[1]);
	Json::Value bitstream;
	if (run_acc)
		bitstream = read_params(argv[2]);

	// heston params
	HestonParamsSL params = get_sl_params(json);
	// reference values
	auto ref = json["reference"];
	double ref_price = ref["price"].asDouble();
	double ref_price_precision = ref["precision"].asDouble();
	std::cout << "REF: result = " << ref_price << std::endl;


	// benchmark
	uint64_t steps = params.step_cnt * params.path_cnt;
#ifdef __unix__
	if (run_acc) {
		auto start_acc = std::chrono::steady_clock::now();
		double result_acc = heston_sl_hw(bitstream, params);
		auto end_acc = std::chrono::steady_clock::now();
		std::cout << "ACC: result = " << result_acc << std::endl;
		std::cout << "ACC: "; print_duration(start_acc, end_acc, steps);
	}
#else
	if (run_acc) {
		std::cerr << "Running accelerator only supported under Linux" << 
				std::endl;
		return -1;
	}
#endif
	if (run_cpu) {
		auto start_cpu = std::chrono::steady_clock::now();
		double result_cpu = heston_sl_cpu<float>(params);
		auto end_cpu = std::chrono::steady_clock::now();
		std::cout << "CPU: result = " << result_cpu << std::endl;
		std::cout << "CPU: "; print_duration(start_cpu, end_cpu, steps);
	}

	std::cout << "done" << std::endl;
	return 0;
}


