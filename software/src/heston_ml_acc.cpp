//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"
#include "json_helper.hpp"
#include "heston_common.hpp"
#include "heston_both_acc.hpp"
#include "heston_ml_both.hpp"

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
#include <list>
#include <cassert>
#include <stdexcept>


struct HestonParamsHWML {
	// call option
	float log_spot_price;
	float reversion_rate_TIMES_step_size_fine;
	float reversion_rate_TIMES_step_size_coarse;
	float long_term_avg_vola;
	float vol_of_vol_TIMES_sqrt_step_size_fine;
	float vol_of_vol_TIMES_sqrt_step_size_coarse;
	float double_riskless_rate; // = 2 * riskless_rate
	float vola_0;
	float correlation;
	// both knockout
	float log_lower_barrier_value;
	float log_upper_barrier_value;
	// simulation params
	uint32_t step_cnt_coarse;
	uint32_t ml_constant; // only 5 bit
	uint32_t do_multilevel; // boolean
	float half_step_size_fine; // = step_size_fine / 2
	float half_step_size_coarse; // = step_size_coarse / 2
	float sqrt_step_size_fine; // = sqrt(step_size_fine)
	float sqrt_step_size_coarse; // = sqrt(step_size_coarse)
	// = BARRIER_HIT_CORRECTION * sqrt_step_size
	float barrier_correction_factor_fine;
	float barrier_correction_factor_coarse;
	uint32_t path_cnt;
};


/**
 * Parses the result stream for one multi-level pricing accelerator
 * and passes them to the pricer
 */
class ResultStreamParser {
public:
	ResultStreamParser(const uint32_t path_cnt_requested, 
			const bool do_multilevel, Pricer<float> &pricer)
			: path_cnt_requested(path_cnt_requested),
			  do_multilevel(do_multilevel), 
			  pricer(pricer) {
	}

	void write(float val) {
		// first word in stream is block size (> 0)
		if (!block_size_valid) {
			block_size = (uint32_t) val;
			block_size_valid = true;
			return;
		}
		if (do_multilevel) {
			if (is_curr_block_fine)
				fine_paths.push_back(val);
			else {
				pricer.handle_path(fine_paths.front(), val);
				fine_paths.pop_front();
			}

			if (++curr_block_position >= block_size) {
				curr_block_position = 0;
				is_curr_block_fine = !is_curr_block_fine;
			}
		} else {
			pricer.handle_path(val);
		}
	}

	/**
	 * Returns the number of words that will be read from
	 * the fifo
	 *
	 * The number of words read depends on block size and
	 * might be bigger than the numbers returned here. Use
	 * the is_total_read_count_final to check if the number
	 * returned will not change. The number returned will never
	 * be smaller than the final number.
	 */
	uint32_t get_total_read_count() {
		uint32_t path_cnt;
		if (block_size_valid)
			path_cnt = round_to_next_block(path_cnt_requested);
		else
			path_cnt = path_cnt_requested;
		return (do_multilevel ? 2 : 1) * path_cnt + 1;
	}

	bool is_total_read_count_final() {
		return block_size_valid;
	}
private:
	uint32_t round_to_next_block(uint32_t i) {
		assert(block_size_valid);
		return ((i + block_size - 1) / block_size) * block_size;
	}

	const uint32_t path_cnt_requested;
	const bool do_multilevel;
	Pricer<float> &pricer;

	uint32_t curr_block_position = 0;
	bool is_curr_block_fine = true;
	std::list<float> fine_paths;

	uint32_t block_size;
	bool block_size_valid = false;
};


Statistics heston_ml_hw_kernel(const Json::Value bitstream, 
		const HestonParamsML ml_params, const uint32_t step_cnt_fine, 
		const uint32_t path_cnt, const bool do_multilevel) {
	HestonParamsML p = ml_params;
	if (p.ml_constant == 0) {
		std::cerr << "ERROR: ml_constant == 0" << std::endl;
		exit(1);
	}
	if (step_cnt_fine % p.ml_constant != 0) {
		std::cerr << "ERROR: step_cnt_fine % ml_constant != 0" << std::endl;
		exit(1);
	}
	// define heston hw parameters
	// continuity correction, see Broadie, Glasserman, Kou (1997)
	float barrier_hit_correction = 0.5826;
	uint32_t step_cnt_coarse = step_cnt_fine / p.ml_constant;
	float step_size_fine = p.time_to_maturity / step_cnt_fine;
	float step_size_coarse = p.time_to_maturity / step_cnt_coarse;
	float sqrt_step_size_fine = std::sqrt(step_size_fine);
	float sqrt_step_size_coarse = std::sqrt(step_size_coarse);
	HestonParamsHWML params_hw = {
		(float) std::log(p.spot_price),
		(float) (p.reversion_rate * step_size_fine),
		(float) (p.reversion_rate * step_size_coarse),
		(float) p.long_term_avg_vola,
		(float) (p.vol_of_vol * sqrt_step_size_fine),
		(float) (p.vol_of_vol * sqrt_step_size_fine),
		(float) (2 * p.riskless_rate),
		(float) p.vola_0,
		(float) p.correlation,
		(float) std::log(p.lower_barrier_value),
		(float) std::log(p.upper_barrier_value),
		step_cnt_coarse,
		p.ml_constant,
		do_multilevel,
		step_size_fine / 2,
		step_size_coarse / 2,
		sqrt_step_size_fine,
		sqrt_step_size_fine,
		(float) (barrier_hit_correction * sqrt_step_size_fine),
		(float) (barrier_hit_correction * sqrt_step_size_coarse),
		0};

	// find accelerators
	std::vector<Json::Value> accelerators;
	std::vector<Json::Value> fifos;
	for (auto component: bitstream)
		if (component["__class__"] == "heston_ml") {
			accelerators.push_back(component);
			fifos.push_back(component["axi_fifo"]);
		}
	if (accelerators.size() == 0)
		throw std::runtime_error("no accelerators found");

	// setup pricer and parser
	Pricer<float> pricer(do_multilevel, p);
	std::vector<ResultStreamParser> parsers;

	// start accelerators
	uint32_t acc_path_cnt = path_cnt / accelerators.size();
	int path_cnt_remainder = path_cnt % accelerators.size();
	for (auto acc: accelerators) {
		params_hw.path_cnt = acc_path_cnt + (path_cnt_remainder-- > 0 ? 1 : 0);
		start_heston_accelerator(acc, &params_hw, sizeof(params_hw));
		parsers.push_back(ResultStreamParser(params_hw.path_cnt, 
				do_multilevel, pricer));
	}

	// setup read iterator
	read_fifos_iterator<float> read_it(fifos);

	// calculate result
	unsigned index;
	float price;
	bool all_sizes_known = false;
	while (!all_sizes_known) {
		// update fifo read count
		all_sizes_known = true;
		uint32_t total_size = 0;
		for (auto parser: parsers) {
			total_size += parser.get_total_read_count();
			all_sizes_known &= parser.is_total_read_count_final();
		}
		read_it.set_new_read_cnt(total_size);

		// read everything from iterator
		while (read_it.next(price, index)) {
			parsers[index].write(price);
		}
	}
	return pricer.get_statistics();
}


float heston_ml_hw(const Json::Value &bitstream, 
		const HestonParamsML &ml_params) {
	auto func_hw = [&bitstream] (const HestonParamsML &ml_params, 
			const uint32_t step_cnt, const uint64_t path_cnt, 
			const bool do_multilevel, const uint32_t) -> Statistics {
		return heston_ml_hw_kernel(bitstream, ml_params, step_cnt, path_cnt, 
				do_multilevel);
	};
	return (float) heston_ml_control(ml_params, func_hw);
}

