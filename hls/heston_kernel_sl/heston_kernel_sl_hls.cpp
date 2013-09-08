//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "heston_kernel_sl_hls.hpp"

#include <hls_math.h>

#include <stdint.h>
#include <limits>

struct state_t {
	calc_t stock;
	calc_t vola;
	bool barrier_hit;
};

// For next interface change
//TODO(brugger): remove step_size
//TODO(brugger): path_cnt should be uint64_t
//TODO(brugger): add correlation
void heston_kernel_sl(
		// call option
		calc_t log_spot_price,
		calc_t reversion_rate_TIMES_step_size,
		calc_t long_term_avg_vola,
		calc_t vol_of_vol_TIMES_sqrt_step_size,
		calc_t double_riskless_rate, // = 2 * riskless_rate
		calc_t vola_0,
//		calc_t correlation,
//		calc_t time_to_maturity,
	    // both knockout
		calc_t log_lower_barrier_value,
		calc_t log_upper_barrier_value,
		// simulation params
		uint32_t step_cnt,
		calc_t step_size, // = time_to_maturity / step_cnt
		calc_t half_step_size, // = step_size / 2
		calc_t sqrt_step_size, // = sqrt(step_size)
		calc_t barrier_correction_factor, // = BARRIER_HIT_CORRECTION * sqrt_step_size
		uint32_t path_cnt,

		hls::stream<calc_t> &gaussian_rn1,
		hls::stream<calc_t> &gaussian_rn2,
		hls::stream<calc_t> &prices)
{
	#pragma HLS interface ap_none port=log_spot_price
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=log_spot_price
	#pragma HLS interface ap_none port=reversion_rate_TIMES_step_size
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=reversion_rate_TIMES_step_size
	#pragma HLS interface ap_none port=long_term_avg_vola
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=long_term_avg_vola
	#pragma HLS interface ap_none port=vol_of_vol_TIMES_sqrt_step_size
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=vol_of_vol_TIMES_sqrt_step_size
	#pragma HLS interface ap_none port=double_riskless_rate
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=double_riskless_rate
	#pragma HLS interface ap_none port=vola_0
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=vola_0
//	#pragma HLS interface ap_none port=correlation
//	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=correlation
//	#pragma HLS interface ap_none port=time_to_maturity
//	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=time_to_maturity
	#pragma HLS interface ap_none port=log_lower_barrier_value
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=log_lower_barrier_value
	#pragma HLS interface ap_none port=log_upper_barrier_value
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=log_upper_barrier_value
	#pragma HLS interface ap_none port=step_cnt
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=step_cnt
	#pragma HLS interface ap_none port=step_size
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=step_size
	#pragma HLS interface ap_none port=half_step_size
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=half_step_size
	#pragma HLS interface ap_none port=sqrt_step_size
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=sqrt_step_size
	#pragma HLS interface ap_none port=barrier_correction_factor
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=barrier_correction_factor
	#pragma HLS interface ap_none port=path_cnt
	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=path_cnt

	#pragma HLS interface ap_fifo port=gaussian_rn1
	#pragma HLS resource core=AXI4Stream variable=gaussian_rn1
	#pragma HLS interface ap_fifo port=gaussian_rn2
	#pragma HLS resource core=AXI4Stream variable=gaussian_rn2
	#pragma HLS interface ap_fifo port=prices
	#pragma HLS resource core=AXI4Stream variable=prices

	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle params" variable=return

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	state_t states[BLOCK_SIZE];
	#pragma HLS data_pack variable=states

	for (uint32_t block = 0; block < path_cnt; block += BLOCK_SIZE) {
		for (uint32_t step = 0; step != step_cnt; ++step) {
			// TODO(brugger): use data type with less bits for inner counter
			for (uint32_t i = 0; i != BLOCK_SIZE; ++i) {
				#pragma HLS PIPELINE II=1

				state_t l_state;
				// initialize
				if (step == 0) {
					l_state.stock = log_spot_price;
					l_state.vola = vola_0;
					l_state.barrier_hit = false;
				} else {
					l_state = states[i];
				}

				// calcualte next step
				state_t n_state;
				calc_t max_vola = MAX((calc_t) 0., l_state.vola);
				calc_t sqrt_vola = hls::sqrtf(max_vola);
				n_state.stock = l_state.stock + (double_riskless_rate - max_vola) *
						half_step_size + sqrt_step_size * sqrt_vola *
						gaussian_rn1.read();
				n_state.vola = l_state.vola + reversion_rate_TIMES_step_size *
						(long_term_avg_vola - max_vola) +
						vol_of_vol_TIMES_sqrt_step_size * sqrt_vola *
						(calc_t) gaussian_rn2.read();
				calc_t barrier_correction = barrier_correction_factor *
						sqrt_vola;
				#pragma HLS RESOURCE variable=barrier_correction \
						core=FMul_meddsp
				n_state.barrier_hit = l_state.barrier_hit |  (n_state.stock <
						log_lower_barrier_value + barrier_correction) |
						(n_state.stock > log_upper_barrier_value -
						barrier_correction);
				states[i] = n_state;

				// write out
				if (step + 1 == step_cnt && (block + i) < path_cnt)
					prices.write(n_state.barrier_hit ?
							-std::numeric_limits<calc_t>::infinity() :
							n_state.stock);

			}
		}
	}
}
