//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "heston_kernel_ml_hls.hpp"

#include "mt19937ar.h"

#include <cmath>
#include <iostream>

// continuity correction, see Broadie, Glasserman, Kou (1997)
#define BARRIER_HIT_CORRECTION 0.5826

void init_rng() {
	init_genrand(0);
}


void box_muller(double &z0, double &z1) {
	double u1 = (genrand_int32() + 1) * (1.0 / 4294967296.0);
	double u2 = (genrand_int32() + 1) * (2.0 * M_PI / 4294967296.0);

	double r = std::sqrt(-2 * std::log(u1));
	z0 = r * std::cos(u2);
	z1 = r * std::sin(u2);
}

void fill_gaussian_rng_stream(hls::stream<calc_t> &rns, unsigned size) {
	double z0, z1;
	for (unsigned i = 0; i < size; ++i) {
		box_muller(z0, z1);
		rns.write(z0);
		if (++i < size)
			rns.write(z1);
	}
}

uint32_t round_to_next_block(uint32_t i) {
	return ((i + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
}

int main(int argc, char *argv[]) {

	// call option
	double spot_price = 100;
	double reversion_rate = 0.5;
	double long_term_avg_vola = 0.04;
	double vol_of_vol = 1;
	double riskless_rate = 0;
	double vola_0 = 0.04;
	double correlation = 0;
	double time_to_maturity = 1;
	double strike_price = 100;
	// both knockout
	double lower_barrier_value = 90;
	double upper_barrier_value = 110;
	// reference
	double ref_price = 0.74870;
	double ref_price_precision = 0.00001;

	unsigned step_cnt = 256; //256;
	unsigned path_cnt = 100000; //10000;

	hls::stream<calc_t> gaussian_rn1;
	fill_gaussian_rng_stream(gaussian_rn1, step_cnt *
			round_to_next_block(path_cnt));
	hls::stream<calc_t> gaussian_rn2;
	fill_gaussian_rng_stream(gaussian_rn2, step_cnt *
			round_to_next_block(path_cnt));
	hls::stream<calc_t> prices;

	double step_size = time_to_maturity / step_cnt;

	heston_kernel_ml(
			std::log(spot_price),
			reversion_rate * step_size,
			long_term_avg_vola,
			vol_of_vol * std::sqrt(step_size),
			2 * riskless_rate,
			vola_0,
//			correlation,
//			time_to_maturity,
			// both knockout
			std::log(lower_barrier_value),
			std::log(upper_barrier_value),
			// simulation params
			step_cnt,
			step_size,
			step_size / 2,
			std::sqrt(step_size),
			BARRIER_HIT_CORRECTION * std::sqrt(step_size),
			path_cnt,

			gaussian_rn1,
			gaussian_rn2,
			prices);

	if (prices.size() != path_cnt) {
		std::cerr << "Wrong number of prices returned." << std::endl;
		return -1;
	}

	double result = 0;
	for (unsigned i = 0; i < path_cnt; ++i) {
		result += std::max(0., std::exp(prices.read()) - strike_price);
	}
	result /= path_cnt;
	result *= std::exp(-riskless_rate * time_to_maturity);

	std::cout << "Result    = " << result << std::endl;
	std::cout << "Reference = " << ref_price << std::endl;

	return 0;
}
