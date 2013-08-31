//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

// Optimizations:
// - pre-calculation
// - using best known gaussion transformation method: ziggurat
// - vectorizable loops (AVX)
// - exploit loop pipelining / blocking
//
// Compiler flags:
// - MSCV: /O2 /arch:AVX /fp:fast
// - GCC on ARM: -O3 -march=native -ffast-math -mfpu=neon
// - GCC on Intel: -O3 -march=native -ffast-math

#define _VARIADIC_MAX 10

#include "heston_sl_cpu.hpp"

#include "ziggurat/gausszig_GSL.hpp"

#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <map>
#include <thread>
#include <mutex>
#include <future>
#include <numeric>


// get separate rng for each thread
std::mt19937 get_thread_rng() {
	static uint32_t rng_cnt = std::random_device()();
	static std::map<std::thread::id, std::mt19937> rng_map;
	static std::mutex m;
	{
		std::lock_guard<std::mutex> lock(m);
		if (rng_map.count(std::this_thread::get_id()) == 0) {
			rng_map[std::this_thread::get_id()] = std::mt19937(rng_cnt);
			++rng_cnt;
		}
		return rng_map[std::this_thread::get_id()];
	}
}


struct HestonParamsSL {
	// call option
	calc_t spot_price;
	calc_t reversion_rate;
	calc_t long_term_avg_vola;
	calc_t vol_of_vol;
	calc_t riskless_rate;
	calc_t vola_0;
	calc_t correlation;
	calc_t time_to_maturity;
	calc_t strike_price;
	// both knowckout
	calc_t lower_barrier_value;
	calc_t upper_barrier_value;
	// simulation params
	uint32_t step_cnt;
	uint32_t path_cnt;
};


// continuity correction, see Broadie, Glasserman, Kou (1997)
#define BARRIER_HIT_CORRECTION 0.5826

#define BLOCK_SIZE 64

// single threaded version
calc_t heston_sl_cpu_kernel(HestonParamsSL p) {
	// pre-compute
	calc_t step_size = p.time_to_maturity / p.step_cnt;
	calc_t sqrt_step_size = std::sqrt(step_size);
	calc_t log_spot_price = std::log(p.spot_price);
	calc_t reversion_rate_TIMES_step_size = p.reversion_rate * step_size;
	calc_t vol_of_vol_TIMES_sqrt_step_size = p.vol_of_vol * sqrt_step_size;
	calc_t double_riskless_rate = 2 * p.riskless_rate;
	calc_t log_lower_barrier_value = std::log(p.lower_barrier_value);
	calc_t log_upper_barrier_value = std::log(p.upper_barrier_value);
	calc_t half_step_size = step_size / 2;
	calc_t barrier_correction_factor = (calc_t)BARRIER_HIT_CORRECTION * 
			sqrt_step_size;
	// having any struct access in our inner most loop prevents
	// msvc to vectorize our code => 6.15s instead of 5.26s (vectorized)
	calc_t long_term_avg_vola = p.long_term_avg_vola;
	// evaluate paths
	if (BLOCK_SIZE % 2 != 0) {
		std::cout << "ERROR: block size has to be even" << std::endl;
		exit(-1);
	}
	double result = 0; // final result
	double t_init = 0, t_steps = 0, t_res = 0;
	std::mt19937 rng = get_thread_rng();
	for (unsigned path = 0; path < p.path_cnt; path += BLOCK_SIZE) {
		// initialize
		calc_t stock[BLOCK_SIZE];
		calc_t vola[BLOCK_SIZE];
		bool barrier_hit[BLOCK_SIZE];
		unsigned upper_i = std::min(p.path_cnt - path, (unsigned) BLOCK_SIZE);
		for (unsigned i = 0; i < upper_i; ++i) {
			stock[i] = log_spot_price;
			vola[i] = p.vola_0;
			barrier_hit[i] = false;
		}
		// calc all steps (takes 99 % of time)
		for (unsigned step = 0; step < p.step_cnt; ++step) {
			// calc random numbers ( takes 70 % of time)
			calc_t z_stock[BLOCK_SIZE];
			calc_t z_vola[BLOCK_SIZE];
			for (unsigned i = 0; i < upper_i; i += 2) {
				float z1 = (calc_t) gsl_ran_gaussian_ziggurat(rng);
				float z2 = (calc_t) gsl_ran_gaussian_ziggurat(rng);
				z_stock[i] = z1;
				z_stock[i + 1] = -z1;
				z_vola[i] = z2;
				z_vola[i + 1] = -z2;
			}
			
			// calculate next step (takes 30 % of time)
			calc_t max_vola[BLOCK_SIZE];
			for (unsigned i = 0; i < upper_i; ++i) 
					max_vola[i] = std::max((calc_t) 0, vola[i]);
			calc_t sqrt_vola[BLOCK_SIZE];
			calc_t barrier_correction[BLOCK_SIZE];
			calc_t upper[BLOCK_SIZE];
			calc_t lower[BLOCK_SIZE];
			// separate vectoriable constructs
			for (unsigned i = 0; i < upper_i; ++i) {
				sqrt_vola[i] = std::sqrt(max_vola[i]);
			}
			for (unsigned i = 0; i < upper_i; ++i) {
				stock[i] += (double_riskless_rate - max_vola[i]) *
						half_step_size + sqrt_step_size * sqrt_vola[i] *
						z_stock[i];
				vola[i] += reversion_rate_TIMES_step_size *
						(long_term_avg_vola - max_vola[i]) +
						vol_of_vol_TIMES_sqrt_step_size * sqrt_vola[i] *
						z_vola[i];
				barrier_correction[i] = barrier_correction_factor *
						sqrt_vola[i];
				lower[i] = log_lower_barrier_value + barrier_correction[i];
				upper[i] = log_upper_barrier_value - barrier_correction[i];
			}
			for (unsigned i = 0; i < upper_i; ++i)
				barrier_hit[i] |= (stock[i] < lower[i]) | (stock[i] > upper[i]);
		}
		// get price of option
		for (unsigned i = 0; i < upper_i; ++i) { //upper_i
			calc_t price = barrier_hit[i] ? 0 : std::exp(stock[i]);
			result += std::max((calc_t)0, price - p.strike_price);
		}
	}
	// payoff price and return
	result *= std::exp(-p.riskless_rate * p.time_to_maturity) / p.path_cnt;
	return (calc_t)result;
}

#define NT 4

calc_t heston_sl_cpu(
		// call option
		calc_t spot_price,
		calc_t reversion_rate,
		calc_t long_term_avg_vola,
		calc_t vol_of_vol,
		calc_t riskless_rate,
		calc_t vola_0,
		calc_t correlation,
		calc_t time_to_maturity,
		calc_t strike_price,
		// both knowckout
		calc_t lower_barrier_value,
		calc_t upper_barrier_value,
		// simulation params
		uint32_t step_cnt,
		uint32_t path_cnt) {
	HestonParamsSL p = {spot_price,
		reversion_rate, long_term_avg_vola, vol_of_vol, riskless_rate,
		vola_0, correlation, time_to_maturity, strike_price, lower_barrier_value,
		upper_barrier_value, step_cnt, path_cnt / NT};
	std::future<calc_t> f[NT];
	for (int i = 0; i < NT; ++i)
		f[i] = std::async(heston_sl_cpu_kernel, p);
	calc_t sum = 0;
	for (int i = 0; i < NT; ++i)
		sum += f[i].get();
	return (calc_t)(sum / NT);
}
