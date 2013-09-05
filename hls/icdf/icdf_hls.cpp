//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 04. September 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "icdf_hls.hpp"

#include <ap_fixed.h>

#include "coeff_lut.hpp"


// return the number of leading zeros of input
ap_uint<4> count_leading_zeros(ap_uint<10> input) {
	bool inc_lz = true;
	ap_uint<4> lz = 0;
	// go through bits from front to back
	for (int i = 1; i <= 10; ++i) {
		// increment as long as there are zeros
		inc_lz &= input[10 - i]  == 0;
		if (inc_lz)
			lz = i;
	}
	return lz;
}


struct InterpolationIndex {
	bool sign;
	ap_uint<6> exponential_segment;
	ap_uint<4> linear_segment;
	ap_uint<17> interpolation_x;
};


// returns the next interpolation index by consumin one or more
// random numbers from the input stream
// 	  converts the first bit to sign
//    the following 10 bits to a 6 bit exponent
//    the following 4 bits to a linear exponent
//	  the last 17 bits are the interpolation position x
InterpolationIndex get_next_interpolation_input(
		hls::stream<uint32_t> &stream_in) {
	ap_uint<6> total_exp_segment = 0;

	#pragma HLS loop_tripcount min=1 max=6 avg=2
	while (true) {
		#pragma HLS PIPELINE II=1
		ap_int<32> input = stream_in.read();

		InterpolationIndex index;
		index.sign = input[31];
		ap_uint<10> exp_bits = input(30, 21);
		index.linear_segment = input(20, 17);
		index.interpolation_x = input;

		ap_uint<4> curr_exp_segment = count_leading_zeros(exp_bits);
		total_exp_segment += curr_exp_segment;
		index.exponential_segment = total_exp_segment;

		if ((curr_exp_segment != 10) || (total_exp_segment == 60)) {
			return index;
		}
	}
}


float set_sign_to(float x, bool sign) {
	ap_uint<32> x_bit = *(reinterpret_cast<ap_uint<32>*>(&x));
	x_bit.set(31, sign);
	float res = *(reinterpret_cast<float*>(&x_bit));
	return res;
}


float icdf_linear_interpolated(InterpolationIndex index) {
	ap_uint<10> table_index = (index.exponential_segment, index.linear_segment);
	InterpolationCoefficients coeffs = coeff_lut[table_index];
	ap_ufixed<17,0> x = *(reinterpret_cast<ap_ufixed<17,0>*>(
			&index.interpolation_x));
	ap_ufixed<30,10> prod = coeffs.coeff_1 * x;
	ap_fixed<30,10> res_fixed = prod - coeffs.coeff_2;
	float res = res_fixed;//set_sign_to((float)res_fixed, index.sign);
	return res;
}


void icdf(
		hls::stream<uint32_t> &uniform_rns,
		hls::stream<float> &gaussian_rns) {
	#pragma HLS interface ap_fifo port=uniform_rns
	#pragma HLS resource core=AXI4Stream variable=uniform_rns
	#pragma HLS interface ap_fifo port=gaussian_rns
	#pragma HLS resource core=AXI4Stream variable=gaussian_rns
	#pragma HLS interface ap_ctrl_none port=return

	for (int i = 0; i < 100; ++i) {
		InterpolationIndex index = get_next_interpolation_input(uniform_rns);
		//float res_float = icdf_linear_interpolated(index);

		// cast to float
		uint32_t unif_float = 0;
		unif_float |= index.exponential_segment;
		float res_float = *(reinterpret_cast<float*>(&unif_float));

		gaussian_rns.write(res_float);
	}

}

