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

	unsigned ml_constant = 4;
	bool do_multilevel = true;
	unsigned step_cnt = 256; //256;
	const unsigned path_cnt = 512; //10000;

	if (step_cnt % ml_constant != 0) {
		std::cerr << "ERROR: step_cnt % ml_constant != 0" << std::endl;
		return 1;
	}
	if (path_cnt % BLOCK_SIZE != 0) {
		std::cerr << "ERROR: path_cnt % BLOCK_SIZE != 0" << std::endl;
		return 1;
	}

	hls::stream<calc_t> gaussian_rn1;
	fill_gaussian_rng_stream(gaussian_rn1, step_cnt *
			round_to_next_block(path_cnt));
	hls::stream<calc_t> gaussian_rn2;
	fill_gaussian_rng_stream(gaussian_rn2, step_cnt *
			round_to_next_block(path_cnt));
	hls::stream<calc_t> prices;

	double step_size_fine = time_to_maturity / step_cnt;
	double step_size_coarse = step_size_fine * ml_constant;

	params_ml params = {
			std::log(spot_price),
			reversion_rate * step_size_fine,
			reversion_rate * step_size_coarse,
			long_term_avg_vola,
			vol_of_vol * std::sqrt(step_size_fine),
			vol_of_vol * std::sqrt(step_size_coarse),
			2 * riskless_rate,
			vola_0,
			correlation,
			// both knockout
			std::log(lower_barrier_value),
			std::log(upper_barrier_value),
			// simulation params
			step_cnt / ml_constant,
			ml_constant,
			do_multilevel,
			//step_size,
			step_size_fine / 2,
			step_size_coarse / 2,
			std::sqrt(step_size_fine),
			std::sqrt(step_size_coarse),
			BARRIER_HIT_CORRECTION * std::sqrt(step_size_fine),
			BARRIER_HIT_CORRECTION * std::sqrt(step_size_coarse),
			path_cnt};

	heston_kernel_ml(params, gaussian_rn1, gaussian_rn2, prices);

	if (prices.size() != path_cnt * 2 + 1) {
		std::cerr << "Wrong number of prices returned." << std::endl;
		std::cerr << "Expected " << path_cnt * 2 + 1 <<
				" got " << prices.size() << std::endl;
		return -1;
	}

	int block_size = prices.read();
	float res_prices[path_cnt * 2];
	for (unsigned i = 0; i < path_cnt * 2; ++i) {
		res_prices[i] = std::max(0., std::exp(prices.read()) - strike_price);
	}

	// sort fine and coarse paths
	double result_fine = 0;
	double result_coarse = 0;
	for (unsigned i = 0; i < path_cnt; ++i) {
		int block = i / block_size;
		int block_i = i % block_size;
		result_fine += res_prices[2 * block * block_size + block_i];
		result_coarse += res_prices[(2 * block + 1) * block_size + block_i];
		std::cout << i << " - " << res_prices[2 * block * block_size + block_i]
		        << " - " << res_prices[(2 * block + 1) * block_size + block_i] << std::endl;
	}
	result_fine /= path_cnt;
	result_coarse /= path_cnt;
	result_fine *= std::exp(-riskless_rate * time_to_maturity);
	result_coarse *= std::exp(-riskless_rate * time_to_maturity);

	std::cout << "Result coarse = " << result_coarse << std::endl;
	std::cout << "Result fine   = " << result_fine << std::endl;
	std::cout << "Reference     = " << ref_price << std::endl;

	return 0;
}
