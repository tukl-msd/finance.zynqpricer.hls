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
 * Accumulates all log prices from all streams and calculates
 * all the multi-level metrics on the fly
 */
class Pricer {
public:
	Pricer(const bool do_multilevel, const HestonParamsML params) 
			: do_multilevel(do_multilevel), params(params) {
	}

	void handle_path(float fine_path, float coarse_path=0) {
		float val = get_payoff(fine_path) - 
				(do_multilevel ? get_payoff(coarse_path) : 0);
		update_online_statistics(val);
	}

	Statistics get_statistics() {
		Statistics stats;
		stats.mean = price_mean;
		stats.variance =price_variance / (price_cnt - 1);
		stats.cnt = price_cnt;
		return stats;
	}
private:
	float get_payoff(float path) {
		return std::max(0.f, std::exp(path) - (float) params.strike_price);
	}

	void update_online_statistics(float val) {
		// See Knuth TAOCP vol 2, 3rd edition, page 232
		++price_cnt;
		double delta = val - price_mean;
		price_mean += delta / price_cnt;
		price_variance += delta * (val - price_mean);
	};

	const bool do_multilevel;
	const HestonParamsML params;

	double price_mean = 0;
	double price_variance = 0;
	uint32_t price_cnt = 0;
};


/**
 * Parses the result stream for one multi-level pricing accelerator
 * and passes them to the pricer
 */
class ResultStreamParser {
public:
	ResultStreamParser(const uint32_t path_cnt_requested, 
			const bool do_multilevel, Pricer &pricer)
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
	Pricer &pricer;

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
	Pricer pricer(do_multilevel, p);
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


/**
 * time step count for specific ml_level
 */
uint32_t get_time_step_cnt(int ml_level, HestonParamsML params) {
	return std::pow(params.ml_constant, ml_level + params.ml_start_level);
}


void print_performance(std::vector<Statistics> stats, 
		HestonParamsML params,
		std::vector<double> durations,
		std::chrono::steady_clock::time_point start, 
		std::chrono::steady_clock::time_point end) {
	double duration = std::chrono::duration<double>(
			end - start).count();
	// print overhead
	double level_duration_sum = 0;
	for (int level = 0; level < stats.size(); ++level) {
		level_duration_sum += durations[level];
	}
	std::cout << "Control overhead are " << duration - level_duration_sum 
			<< "seconds" << std::endl;
	std::cout << std::endl;
	// print steps / sec
	uint64_t steps = 0;
	for (int level = 0; level < stats.size(); ++level) {
		uint64_t l_steps = (uint64_t) stats[level].cnt * 
				get_time_step_cnt(level, params) *
				(level == 0 ? 1 : (1 + 1. / params.ml_constant));
		std::cout << "Level " << level << " calculated " << l_steps 
				<< " steps in " << durations[level] << " seconds (" << 
				l_steps / durations[level] << " steps / sec)"
				<< std::endl;
		steps += l_steps;
	}
	std::cout << "Calculated " << steps << " steps in " << duration
			<< " seconds (" << steps / duration << " steps / sec)"
			<< std::endl;
	std::cout << std::endl;
	// print bytes / sec
	uint64_t bytes = 0;
	for (int level = 0; level < stats.size(); ++level) {
		uint64_t l_bytes = 4 * ((uint64_t) stats[level].cnt *
				(level == 0 ? 1 : 2) + 1);
		std::cout << "Level " << level << " transmitted " << l_bytes 
				<< " bytes in " << durations[level] << " seconds (" << 
				l_bytes / durations[level] << " bytes / sec)"
				<< std::endl;
		bytes += l_bytes;
	}
	std::cout << "Calculated " << bytes << " bytes in " << duration
			<< " seconds (" << bytes / duration << " bytes / sec)"
			<< std::endl;
	std::cout << std::endl;
}


/**
 * multilevel control, 
 * decides how many paths should be evaluated on which level and 
 * accumulates the results
 */
float heston_ml_hw(const Json::Value &bitstream, 
		const HestonParamsML &ml_params) {
	auto start_f = std::chrono::steady_clock::now();

	int current_level = 0;
	bool first_iteration_on_level = true;
	// Stores number of paths already generated for each level.
	std::vector<int64_t> path_cnt_done;
	// Stores how many paths should be generated on each level.
	// These values are estimated by the multi-level algorithm.
	std::vector<int64_t> path_cnt_opt;
	// Stores the statistic of each multi-level component.
	std::vector<Statistics> stats;
	// stores timing information
	std::vector<double> durations;

	while (true) {
		if (first_iteration_on_level) {
			path_cnt_opt.push_back(ml_params.ml_path_cnt_start);
			stats.push_back(Statistics());
			durations.push_back(0);
		}

		for (int level = 0; level <= current_level; ++level) {
			int64_t path_cnt_todo = path_cnt_opt[level] - stats[level].cnt;
			if (path_cnt_todo > 0) {
				uint32_t step_cnt = get_time_step_cnt(level, ml_params);
				bool do_multilevel = (level > 0);
				std::cout << "current_level " << current_level <<
						" level " << level << " step_cnt " << 
						step_cnt << " path_cnt " << 
						path_cnt_todo << std::endl;
				auto start = std::chrono::steady_clock::now();
				Statistics new_stats = heston_ml_hw_kernel(bitstream, 
						ml_params, step_cnt, path_cnt_todo, do_multilevel);
				auto end = std::chrono::steady_clock::now();
				durations[level] += std::chrono::duration<double>(
						end - start).count();

				// update variance and mean
				stats[level] += new_stats;
				std::cout << new_stats << std::endl;
			}
		}

		if (first_iteration_on_level) {
			first_iteration_on_level = false;
			for (int level = 0; level <= current_level; ++level) {
				double sum = 0;
				for (int l = 0; l <= level; ++l)
					sum += sqrt(stats[l].variance / (ml_params.time_to_maturity /
							get_time_step_cnt(l, ml_params)));
				path_cnt_opt[level] = std::ceil(2 * 
						std::pow(ml_params.ml_epsilon, -2) * sum * 
						sqrt(stats[level].variance * ml_params.time_to_maturity /
						get_time_step_cnt(level, ml_params)));
			}
		} else {
			if ((current_level > 2) && (std::max(1. / ml_params.ml_constant * 
					abs(stats[current_level - 1].mean), 
					stats[current_level].mean) < ml_params.ml_epsilon / 
					std::sqrt(2) * (ml_params.ml_constant - 1)))
				break;
			current_level += 1;
			first_iteration_on_level = true;
		}
	}

	double sum = 0;
	for (auto s: stats)
		sum += s.mean;
	double r = sum * exp(-ml_params.riskless_rate * ml_params.time_to_maturity);

	auto end_f = std::chrono::steady_clock::now();
	print_performance(stats, ml_params, durations, start_f, end_f);
	return r;
}


