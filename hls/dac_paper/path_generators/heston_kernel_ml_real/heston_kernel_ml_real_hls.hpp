//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 7. September 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#ifndef __HESTON_KERNEL_SL_HLS_HPP__
#define __HESTON_KERNEL_SL_HLS_HPP__

#include <hls_stream.h>
#include <ap_int.h>

#include <stdint.h>

#define BLOCK_SIZE 256

#define calc_t float

struct params_ml {
	calc_t spot_price;
	calc_t reversion_rate_TIMES_step_size_fine;
	calc_t reversion_rate_TIMES_step_size_coarse;
	calc_t long_term_avg_vola;
	calc_t vol_of_vol_TIMES_sqrt_step_size_fine;
	calc_t double_riskless_rate; // = 2 * riskless_rate
	calc_t vola_0;
	calc_t correlation;
	calc_t inv_correlation; // = sqrt(1 - correlation**2)
	// both knockout
//	calc_t log_lower_barrier_value;
//	calc_t log_upper_barrier_value;
	// simulation params
	uint32_t step_cnt_coarse; // assert step_cnt % ml_constant == 0
	ap_uint<5> ml_constant;
	bool do_multilevel;
	// step_size = time_to_maturity / step_cnt
	calc_t half_step_size_fine; // = step_size_fine / 2
	calc_t half_step_size_coarse; // = step_size_coarse / 2
	calc_t sqrt_step_size_fine; // = sqrt(step_size_fine)
//	calc_t barrier_correction_factor_fine; // = BARRIER_HIT_CORRECTION * sqrt_step_size_fine
//	calc_t barrier_correction_factor_coarse; // = BARRIER_HIT_CORRECTION * sqrt_step_size_coarse
	uint32_t path_cnt;
};

void heston_kernel_ml_real(params_ml params, hls::stream<calc_t> &gaussian_rn1,
		hls::stream<calc_t> &gaussian_rn2, hls::stream<calc_t> &prices);

#endif





