//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 8. September 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "heston_kernel_ml_log_hls.hpp"

#include <hls_math.h>

#include <stdint.h>
#include <limits>

struct state_t {
	calc_t stock;
	calc_t vola;
//	bool barrier_hit;
};

struct w_both_t {
	calc_t w_stock;
	calc_t w_vola;
};

state_t get_init_state(const params_ml params) {
	state_t state;
	state.stock = params.log_spot_price;
	state.vola = params.vola_0;
//	state.barrier_hit = false;
	return state;
}

w_both_t get_w_zero() {
	w_both_t zero = {0, 0};
	return zero;
}

state_t get_next_step(const params_ml params, const state_t l_state,
		const calc_t w_stock, const calc_t w_vola, bool do_fine) {
	state_t n_state;
	calc_t max_vola = MAX((calc_t) 0., l_state.vola);
	calc_t sqrt_vola = hls::sqrtf(max_vola);
	n_state.stock = l_state.stock + (params.double_riskless_rate - max_vola) *
			(do_fine ? params.half_step_size_fine : params.half_step_size_coarse) +
			params.sqrt_step_size_fine * sqrt_vola * w_stock;
	n_state.vola = l_state.vola +
			(do_fine ? params.reversion_rate_TIMES_step_size_fine :
					params.reversion_rate_TIMES_step_size_coarse) *
			(params.long_term_avg_vola - max_vola) +
			params.vol_of_vol_TIMES_sqrt_step_size_fine * sqrt_vola * w_vola;
//	calc_t barrier_correction = sqrt_vola *
//			(do_fine ? params.barrier_correction_factor_fine :
//					params.barrier_correction_factor_coarse);
//	n_state.barrier_hit = l_state.barrier_hit ||  (n_state.stock <
//			params.log_lower_barrier_value + barrier_correction) ||
//			(n_state.stock > params.log_upper_barrier_value -
//			barrier_correction);
	return n_state;
}

calc_t get_log_price(state_t state) {
//	if (state.barrier_hit)
//		return -std::numeric_limits<calc_t>::infinity();
//	else
		return state.stock;
}

void heston_kernel_ml_log(const params_ml params, hls::stream<calc_t> &gaussian_rn1,
		hls::stream<calc_t> &gaussian_rn2, hls::stream<calc_t> &prices) {
	#pragma HLS interface ap_none port=params
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" \
			variable=params
	#pragma HLS interface ap_fifo port=gaussian_rn1
	#pragma HLS resource core=AXI4Stream variable=gaussian_rn1
	#pragma HLS interface ap_fifo port=gaussian_rn2
	#pragma HLS resource core=AXI4Stream variable=gaussian_rn2
	#pragma HLS interface ap_fifo port=prices
	#pragma HLS resource core=AXI4Stream variable=prices
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" \
			variable=return

	// write block size to stream
	prices.write(BLOCK_SIZE);

	state_t state_coarse[BLOCK_SIZE];
	#pragma HLS data_pack variable=state_coarse
	state_t state_fine[BLOCK_SIZE];
	#pragma HLS data_pack variable=state_fine
	w_both_t w_both[BLOCK_SIZE];
	#pragma HLS data_pack variable=w_both

	ap_uint<6> upper_j = params.ml_constant + (params.do_multilevel ? 1 : 0);

	for (uint32_t path = 0; path < params.path_cnt; path += BLOCK_SIZE) {
		for (uint32_t step = 0; step != params.step_cnt_coarse; ++step) {
			for (ap_uint<6> j = 0; j != upper_j; ++j) {
				for (ap_uint<10> block_i = 0; block_i != BLOCK_SIZE;
						++block_i) {
					#pragma HLS PIPELINE II=1

					bool is_fine = j != params.ml_constant;

					//
					// initialize
					//
					state_t l_state_coarse, l_state_fine;
					w_both_t l_w_both;
					if (step == 0 && j == 0) {
						l_state_coarse = get_init_state(params);
						l_state_fine = get_init_state(params);
						l_w_both = get_w_zero();
					} else {
						l_state_coarse = state_coarse[block_i];
						l_state_fine = state_fine[block_i];
						l_w_both = w_both[block_i];
					}

					//
					// calculate next step
					//
					state_t n_state_coarse, n_state_fine;
					w_both_t n_w_both;

					if (is_fine) {
						// step fine
						calc_t w_stock = gaussian_rn1.read();
						calc_t w_vola = params.correlation * w_stock +
								params.inv_correlation * gaussian_rn2.read();
						n_state_fine = get_next_step(params, l_state_fine,
								w_stock, w_vola, true);
						n_state_coarse = l_state_coarse;
						// accumulate random numbers
						n_w_both.w_stock = l_w_both.w_stock + w_stock;
						n_w_both.w_vola = l_w_both.w_vola + w_vola;
					} else {
						// step coarse
						n_state_coarse = get_next_step(params, l_state_coarse,
								l_w_both.w_stock, l_w_both.w_vola, false);
						n_state_fine = l_state_fine;
						n_w_both = get_w_zero();
					}

					state_coarse[block_i] = n_state_coarse;
					state_fine[block_i] = n_state_fine;
					w_both[block_i] = n_w_both;

					//
					// write out
					//
					if ((step + 1 == params.step_cnt_coarse) &&
							(j + 1 >= params.ml_constant)) {
						if (is_fine) {
							prices.write(get_log_price(n_state_fine));
						} else {
							prices.write(get_log_price(n_state_coarse));
						}
					}
				}
			}
		}
	}
}
