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
		std::chrono::steady_clock::time_point end, unsigned steps) {
	double duration = std::chrono::duration<double>(
		end - start).count();
	std::cout << "Calculated " << steps << " steps in " << duration
		<< " seconds (" << steps / duration << " steps / sec)"
		<< std::endl;
}


void check_usage(int argc, char *argv0, bool run_acc) {
	if ((run_acc and argc != 3) or (!run_acc and argc != 2)) {
		std::cerr << "Usage: " << argv0 << " params.json" << 
				(run_acc ? " bitstream.json" : "") << std::endl;
		exit(-1);
	}
}


int main_runner(int argc, char *argv[], bool run_cpu, bool run_acc)
{
	check_usage(argc, argv[0], run_acc);

	Json::Value params = read_params(argv[1]);
	Json::Value bitstream;
	if (run_acc)
		bitstream = read_params(argv[2]);

	// heston params
	auto heston = params["heston"];
	float spot_price = heston["spot_price"].asFloat();
	float reversion_rate = heston["reversion_rate"].asFloat();
	float long_term_avg_vola = heston["long_term_avg_vola"].asFloat();
	float vol_of_vol = heston["vol_of_vol"].asFloat();
	float riskless_rate = heston["riskless_rate"].asFloat();
	float vola_0 = heston["vola_0"].asFloat();
	float correlation = heston["correlation"].asFloat();
	float time_to_maturity = heston["time_to_maturity"].asFloat();
	float strike_price = heston["strike_price"].asFloat();
	// both knowckout
	auto barrier = params["barrier values"];
	float lower_barrier_value = barrier["lower"].asFloat();
	float upper_barrier_value = barrier["upper"].asFloat();
	// simulation params
	auto simulation = params["simulation"];
	uint32_t step_cnt = simulation["step_cnt"].asUInt();
	uint32_t path_cnt = simulation["path_cnt"].asUInt();
	// reference values
	auto ref = params["reference"];
	float ref_price = ref["price"].asFloat();
	float ref_price_precision = ref["precision"].asFloat();
	std::cout << "REF: result = " << ref_price << std::endl;


	// benchmark
	uint32_t steps = step_cnt * path_cnt;
#ifdef __unix__
	if (run_acc) {
		auto start_acc = std::chrono::steady_clock::now();
		float result_acc = heston_sl_hw(bitstream, spot_price, reversion_rate,
			long_term_avg_vola, vol_of_vol, riskless_rate, vola_0,
			correlation, time_to_maturity, strike_price, lower_barrier_value,
			upper_barrier_value, step_cnt, path_cnt);
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
		float result_cpu = heston_sl_cpu<float>(spot_price, reversion_rate,
			long_term_avg_vola, vol_of_vol, riskless_rate, vola_0,
			correlation, time_to_maturity, strike_price, lower_barrier_value,
			upper_barrier_value, step_cnt, path_cnt);
		auto end_cpu = std::chrono::steady_clock::now();
		std::cout << "CPU: result = " << result_cpu << std::endl;
		std::cout << "CPU: "; print_duration(start_cpu, end_cpu, steps);
	}

	std::cout << "done" << std::endl;
	return 0;
}


