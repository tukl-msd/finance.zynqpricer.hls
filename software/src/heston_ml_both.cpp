//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#include "heston_ml_both.hpp"

#include <stdint.h>

#include <vector>
#include <chrono>
#include <cmath>


Pricer::Pricer(const bool do_multilevel, const HestonParams params) 
		: do_multilevel(do_multilevel), params(params),
			price_mean(0), price_variance(0), price_cnt(0) {
}

void Pricer::handle_path(float fine_path, float coarse_path) {
	float val = get_payoff(fine_path) - 
			(do_multilevel ? get_payoff(coarse_path) : 0);
	update_online_statistics(val);
}

Statistics Pricer::get_statistics() {
	Statistics stats;
	stats.mean = price_mean;
	stats.variance =price_variance / (price_cnt - 1);
	stats.cnt = price_cnt;
	return stats;
}

float Pricer::get_payoff(float path) {
	return std::max(0.f, std::exp(path) - (float) params.strike_price);
}

void Pricer::update_online_statistics(float val) {
	// See Knuth TAOCP vol 2, 3rd edition, page 232
	++price_cnt;
	double delta = val - price_mean;
	price_mean += delta / price_cnt;
	price_variance += delta * (val - price_mean);
};


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
double heston_ml_control(const HestonParamsML &ml_params,
		std::function<Statistics(const HestonParamsML, const uint32_t, 
		const uint64_t, const bool, const uint32_t)> ml_kernel) {
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
				Statistics new_stats = ml_kernel(ml_params, step_cnt, 
						path_cnt_todo, do_multilevel, ml_params.ml_constant);
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
