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
// MSCV: /O2 /arch:AVX /fp:fast
// GCC: -Ofast -march=native


#include "heston_sl_cpu.hpp"

#include "mt19937ar.h"
#include "ziggurat/gausszig_GSL.hpp"

#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

// continuity correction, see Broadie, Glasserman, Kou (1997)
#define BARRIER_HIT_CORRECTION 0.5826

#define BLOCK_SIZE 64

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
		uint32_t path_cnt)
{
	// pre-compute
	calc_t step_size = time_to_maturity / step_cnt;
	calc_t sqrt_step_size = std::sqrt(step_size);
	calc_t log_spot_price = std::log(spot_price);
	calc_t reversion_rate_TIMES_step_size = reversion_rate * step_size;
	calc_t vol_of_vol_TIMES_sqrt_step_size = vol_of_vol * sqrt_step_size;
	calc_t double_riskless_rate = 2 * riskless_rate;
	calc_t log_lower_barrier_value = std::log(lower_barrier_value);
	calc_t log_upper_barrier_value = std::log(upper_barrier_value);
	calc_t half_step_size = step_size / 2;
	calc_t barrier_correction_factor = (calc_t)BARRIER_HIT_CORRECTION * 
			sqrt_step_size;
	// evaluate paths
	if (BLOCK_SIZE % 2 != 0) {
		std::cout << "ERROR: block size has to be even" << std::endl;
		exit(-1);
	}
	double result = 0; // final result
	double t_init = 0, t_steps = 0, t_res = 0;
	for (unsigned path = 0; path < path_cnt; path += BLOCK_SIZE) {
		// initialize
		calc_t stock[BLOCK_SIZE];
		calc_t vola[BLOCK_SIZE];
		bool barrier_hit[BLOCK_SIZE];
		unsigned upper_i = std::min(path_cnt - path, (unsigned) BLOCK_SIZE);
		for (unsigned i = 0; i < upper_i; ++i) {
			stock[i] = log_spot_price;
			vola[i] = vola_0;
			barrier_hit[i] = false;
		}
		// calc all steps (takes 99 % of time)
		for (unsigned step = 0; step < step_cnt; ++step) {
			// calc random numbers ( takes 70 % of time)
			calc_t z_stock[BLOCK_SIZE];
			calc_t z_vola[BLOCK_SIZE];
			for (unsigned i = 0; i < upper_i; i += 2) {
				float z1 = (calc_t) gsl_ran_gaussian_ziggurat();
				float z2 = (calc_t) gsl_ran_gaussian_ziggurat();
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
			result += std::max((calc_t)0, price - strike_price);
		}
	}
	// payoff price and return
	result *= std::exp(-riskless_rate * time_to_maturity) / path_cnt;
	return (calc_t)result;
}

