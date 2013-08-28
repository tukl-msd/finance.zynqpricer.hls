//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#include "heston_sl_cpu.hpp"

#include "mt19937ar.h"
#include "ziggurat/gausszig_GSL.hpp"

#include <iostream>
#include <cmath>
#include <random>
#include <chrono>

#define PI 3.141592653589793

std::random_device rd;
std::mt19937 rng(rd());
std::normal_distribution<calc_t> g_rn;


void box_muller_1(calc_t &z0, calc_t &z1) {
	//CPU: Calculated 1000000 values in 7.53385 seconds (132734 values / sec)
	calc_t u1 = (genrand_int32() + 1) * (calc_t)(1.0 / 4294967296.0);
	calc_t u2 = (genrand_int32() + 1) * (calc_t)(2.0 * PI / 4294967296.0);

	calc_t r = std::sqrt(std::log(u1) * (calc_t)(-2));
	z0 = r * std::cos(u2);
	z1 = r * std::sin(u2);
}

void box_muller_2(calc_t &z0, calc_t &z1) {
	// CPU: Calculated 1000000 values in 8.58766 seconds (116446 values / sec)
	// CPU: Calculated 1000000 values in 8.5661 seconds (116739 values / sec)
	z0 = g_rn(rng);
	z1 = g_rn(rng);
}

void box_muller_3(calc_t &z0, calc_t &z1) {
	// CPU: Calculated 1000000 values in 7.26148 seconds (137713 values / sec)
	calc_t u1 = (rng() + 1) * (calc_t)(1.0 / 4294967296.0);
	calc_t u2 = (rng() + 1) * (calc_t)(2.0 * PI / 4294967296.0);

	calc_t r = std::sqrt(std::log(u1) * (calc_t)(-2));
	z0 = r * std::cos(u2);
	z1 = r * std::sin(u2);
}

void box_muller_4(calc_t &z0, calc_t &z1) {
	// CPU: Calculated 1000000 values in 7.27217 seconds (137511 values / sec)
	float x1, x2, w, y1, y2;
 
	do {
		x1 = 2.0f * rng() - 1.0f;
		x2 = 2.0f * rng() - 1.0f;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0f );

	w = sqrt( (-2.0f * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;
}

void box_muller_5(calc_t &z0, calc_t &z1) {
	// CPU: Calculated 1000000 values in 4.83549 seconds (206804 values / sec)
	z0 = (calc_t) gsl_ran_gaussian_ziggurat();
	z1 = (calc_t) gsl_ran_gaussian_ziggurat();
}

void init_cpu() {
	// init rng
	init_genrand(0);
}

void print_dt(std::chrono::steady_clock::time_point start, 
		std::chrono::steady_clock::time_point end) {
	double duration = std::chrono::duration<double>(
		end - start).count();
	std::cout << duration << " seconds " << std::endl;
}

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
//			auto t0 = std::chrono::steady_clock::now();
			// calc random numbers ( takes 59 % of time)
			calc_t z_stock[BLOCK_SIZE];
			calc_t z_vola[BLOCK_SIZE];

			for (unsigned i = 0; i < upper_i; i += 2) {
				//float z1 = g_rn(rng);
				//float z2 = g_rn(rng);
				float z1, z2;
				box_muller_5(z1, z2);
				z_stock[i] = z1;
				z_stock[i + 1] = -z1;
				z_vola[i] = z2;
				z_vola[i + 1] = -z2;
			}
//			auto t1 = std::chrono::steady_clock::now();
			// calculate next step (takes 41 % of time)
			for (unsigned i = 0; i < upper_i; ++i) { // upper_i
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
//			auto t2 = std::chrono::steady_clock::now();
//			t_init += std::chrono::duration<double>(t1 - t0).count();
//			t_steps += std::chrono::duration<double>(t2 - t1).count();
			//t_res += std::chrono::duration<double>(t3 - t2).count();
		}
		// get price of option
		for (unsigned i = 0; i < upper_i; ++i) { //upper_i
			calc_t price = barrier_hit[i] ? 0 : std::exp(stock[i]);
			result += std::max((calc_t)0, price - strike_price);
		}
	}
//	std::cout << "init " << t_init << std::endl;
//	std::cout << "steps " << t_steps << std::endl;
//	std::cout << "res " << t_res << std::endl;
//	std::cout << "res " << t_init / (t_init + t_steps) << std::endl;
//	std::cout << "res " << t_steps / (t_init + t_steps) << std::endl;
	// payoff price and return
	result *= std::exp(-riskless_rate * time_to_maturity) / path_cnt;
	return (calc_t)result;
}

