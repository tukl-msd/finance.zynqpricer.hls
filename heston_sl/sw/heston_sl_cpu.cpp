//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#include "heston_sl_cpu.hpp"

#include "mt19937ar.h"

#include <iostream>
#include <cmath>
#include <random>

#define PI 3.141592653589793

std::random_device rd;
std::mt19937 rng(rd());
std::normal_distribution<calc_t> g_rn;


void box_muller(calc_t &z0, calc_t &z1) {
	/*
	calc_t u1 = (genrand_int32() + 1) * (calc_t)(1.0 / 4294967296.0);
	calc_t u2 = (genrand_int32() + 1) * (calc_t)(2.0 * PI / 4294967296.0);

	calc_t r = std::sqrt(std::log(u1) * (calc_t)(-2));
	z0 = r * std::cos(u2);
	z1 = r * std::sin(u2);
	*/
	z0 = g_rn(rng);
	z1 = g_rn(rng);
}


void init_cpu() {
	// init rng
	init_genrand(0);
}


// continuity correction, see Broadie, Glasserman, Kou (1997)
#define BARRIER_HIT_CORRECTION 0.5826


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
	double result = 0; // final result
	for (unsigned path = 0; path < path_cnt; path += 2) {
		// initialize
		calc_t stock[2] = {log_spot_price, log_spot_price};
		calc_t vola[2] = {vola_0, vola_0};
		bool barrier_hit[2] = {false, false};
		// calc all steps
		int upper_i = (path + 1 < path_cnt) ? 2 : 1;
		for (unsigned step = 0; step < step_cnt; ++step) {
			// get random numbers
			calc_t z_stock[2];
			calc_t z_vola[2];
			box_muller(z_stock[0], z_vola[0]);
			z_stock[1] = -z_stock[0];
			z_vola[1] = -z_vola[0];
			for (int i = 0; i < upper_i; ++i) {
				calc_t max_vola = std::max((calc_t)0, vola[i]);
				calc_t sqrt_vola = std::sqrt(max_vola);
				stock[i] += (double_riskless_rate - max_vola) *
					half_step_size + sqrt_step_size * sqrt_vola *
					z_stock[i];
				vola[i] += reversion_rate_TIMES_step_size *
					(long_term_avg_vola - max_vola) +
					vol_of_vol_TIMES_sqrt_step_size * sqrt_vola *
					z_vola[i];
				calc_t barrier_correction = barrier_correction_factor *
					sqrt_vola;
				barrier_hit[i] |= (stock[i] <
					log_lower_barrier_value + barrier_correction) |
					(stock[i] > log_upper_barrier_value -
					barrier_correction);
			}
		}
		// get price of option
		for (int i = 0; i < upper_i; ++i) {
			calc_t price = barrier_hit[i] ? 0 : std::exp(stock[i]);
			result += std::max((calc_t)0, price - strike_price);
		}
	}
	// payoff price and return
	result *= std::exp(-riskless_rate * time_to_maturity) / path_cnt;
	return (calc_t)result;
}

