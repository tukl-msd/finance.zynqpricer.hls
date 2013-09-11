//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"
#include "json_helper.hpp"
#include "heston_both_acc.hpp"

#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cmath>
#include <fstream>


struct HestonParamsHWSL {
	// call option
	float log_spot_price;
	float reversion_rate_TIMES_step_size;
	float long_term_avg_vola;
	float vol_of_vol_TIMES_sqrt_step_size;
	float double_riskless_rate; // = 2 * riskless_rate
	float vola_0;
    	// both knockout
	float log_lower_barrier_value;
	float log_upper_barrier_value;
	// simulation params
	uint32_t step_cnt;
	float step_size; // = time_to_maturity / step_cnt
	float half_step_size; // = step_size / 2
	float sqrt_step_size; // = sqrt(step_size)
	// = BARRIER_HIT_CORRECTION * sqrt_step_size
	float barrier_correction_factor; 
	uint32_t path_cnt;
};


float heston_sl_hw(Json::Value bitstream, HestonParamsSL sl_params) {
	HestonParamsSL p = sl_params;
	// define heston hw parameters
	// continuity correction, see Broadie, Glasserman, Kou (1997)
	float barrier_hit_correction = 0.5826;
	float step_size = p.time_to_maturity / p.step_cnt;
	float sqrt_step_size = std::sqrt(step_size);
	HestonParamsHWSL params_hw = {
		(float) std::log(p.spot_price),
		(float) (p.reversion_rate * step_size),
		(float) (p.long_term_avg_vola),
		(float) (p.vol_of_vol * sqrt_step_size),
		(float) (2 * p.riskless_rate),
		(float) p.vola_0,
		(float) std::log(p.lower_barrier_value),
		(float) std::log(p.upper_barrier_value),
		p.step_cnt,
		step_size,
		step_size / 2,
		sqrt_step_size,
		(float) (barrier_hit_correction * sqrt_step_size),
		(uint32_t) p.path_cnt};

	// find accelerators
	std::vector<Json::Value> accelerators;
	std::vector<Json::Value> fifos;
	for (auto component: bitstream)
		if (component["__class__"] == "heston_sl") {
			accelerators.push_back(component);
			fifos.push_back(component["axi_fifo"]);
		}

	// start accelerators
	uint32_t acc_path_cnt = p.path_cnt / accelerators.size();
	int path_cnt_remainder = p.path_cnt % accelerators.size();
	for (auto acc: accelerators) {
		params_hw.path_cnt = acc_path_cnt + (path_cnt_remainder-- > 0 ? 1 : 0);
		start_heston_accelerator(acc, &params_hw, sizeof(params_hw));
	}

	// setup read iterator
	read_fifos_iterator<float> read_it(fifos, p.path_cnt);

	// calculate result
	double result = 0;
	float price;
	while (read_it.next(price)) {
		result += std::max(0.f, std::exp(price) - (float) p.strike_price);
	}
	result *= std::exp(-p.riskless_rate * p.time_to_maturity) / p.path_cnt;
	return result;
}

