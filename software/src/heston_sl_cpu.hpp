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
// - MSCV: /O2 /arch:AVX /fp:fast /GL
// - GCC on ARM: -O3 -march=native -ffast-math -mfpu=neon
// - GCC on Intel: -O3 -march=native -ffast-math

#ifndef __HESTON_SL_CPU_HPP__
#define __HESTON_SL_CPU_HPP__

#include "ziggurat/gausszig_GSL.hpp"

#include <stdint.h>

#include <random>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <future>

#include "heston_types.hpp"

std::mt19937 get_thread_rng();

// single threaded version
template<typename calc_t, int BLOCK_SIZE>
calc_t heston_sl_cpu_kernel(HestonParamsSL p) {
	// pre-compute
	// continuity correction, see Broadie, Glasserman, Kou (1997)
	calc_t barrier_hit_correction = 0.5826;
	calc_t step_size = p.time_to_maturity / p.step_cnt;
	calc_t sqrt_step_size = std::sqrt(step_size);
	calc_t log_spot_price = std::log(p.spot_price);
	calc_t reversion_rate_TIMES_step_size = p.reversion_rate * step_size;
	calc_t vol_of_vol_TIMES_sqrt_step_size = p.vol_of_vol * sqrt_step_size;
	calc_t double_riskless_rate = 2 * p.riskless_rate;
	calc_t log_lower_barrier_value = std::log(p.lower_barrier_value);
	calc_t log_upper_barrier_value = std::log(p.upper_barrier_value);
	calc_t half_step_size = step_size / 2;
	calc_t barrier_correction_factor = barrier_hit_correction * sqrt_step_size;
	// having any struct access in our inner most loop prevents
	// msvc to vectorize our code: 6.15s instead of 5.26s (vectorized)
	calc_t long_term_avg_vola = p.long_term_avg_vola;
	// evaluate paths
	if (BLOCK_SIZE % 2 != 0) {
		std::cout << "ERROR: block size has to be even" << std::endl;
		exit(-1);
	}
	double result = 0; // final result
	std::mt19937 rng = get_thread_rng();
	for (uint64_t path = 0; path < p.path_cnt; path += BLOCK_SIZE) {
		// initialize
		calc_t stock[BLOCK_SIZE];
		calc_t vola[BLOCK_SIZE];
		bool barrier_hit[BLOCK_SIZE];
		unsigned upper_i = std::min(p.path_cnt - path, (uint64_t) BLOCK_SIZE);
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
				calc_t z1 = (calc_t) Ziggurat<calc_t>::
						gsl_ran_gaussian_ziggurat(rng);
				calc_t z2 = (calc_t) Ziggurat<calc_t>::
						gsl_ran_gaussian_ziggurat(rng);
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
			result += std::max((calc_t)0, price - (calc_t)p.strike_price);
		}
	}
	// payoff price and return
	result *= std::exp(-p.riskless_rate * p.time_to_maturity) / p.path_cnt;
	return (calc_t)result;
}

template<typename calc_t>
calc_t heston_sl_cpu(HestonParamsSL p) {
#ifndef WITH_MPI
	int nt = std::thread::hardware_concurrency();
	p.path_cnt /= nt;
	std::vector<std::future<calc_t> > f;
	for (int i = 0; i < nt; ++i) {
		f.push_back(std::async(std::launch::async, 
					heston_sl_cpu_kernel<calc_t, 64>, p));
	}
	calc_t sum = 0;
	for (int i = 0; i < nt; ++i)
		sum += f[i].get();
	return (calc_t)(sum / nt);
#else
	return heston_sl_cpu_kernel<calc_t, 64>(p);
#endif
}

#endif
