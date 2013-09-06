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
	bool is_valid;
	bool sign;
	ap_uint<6> exponential_segment;
	ap_uint<4> linear_segment;
	ap_uint<17> interpolation_x;
};


// returns the next interpolation index.
// 	  converts the first bit to sign
//    the following 10 bits to a 6 bit exponent
//    the following 4 bits to a linear exponent
//	  the last 17 bits are the interpolation position x
// This method may have to be called multiple times, to yield a valid
// interpolation index.
InterpolationIndex get_next_interpolation_input(uint32_t next_number) {
	static ap_uint<6> total_exp_segment = 0;

	ap_int<32> input = next_number;

	InterpolationIndex index;
	index.sign = input[31];
	ap_uint<10> exp_bits = input(30, 21);
	index.linear_segment = input(20, 17);
	index.interpolation_x = input;

	ap_uint<4> curr_exp_segment = count_leading_zeros(exp_bits);
	total_exp_segment += curr_exp_segment;
	index.exponential_segment = total_exp_segment;

	index.is_valid = ((curr_exp_segment != 10) || (total_exp_segment == 60));
	if (index.is_valid)
		total_exp_segment = 0;
	return index;
}


float fixed_to_float(ap_ufixed<32,4> fixed, bool sign) {
	// int32 --> float conversion (efficient ip core exists for that)
	ap_uint<32> fixed_bits = *(reinterpret_cast<ap_uint<32>*>(&fixed));
	float res_int = fixed_bits;
	// substract 28 from exponent and set sign
	ap_uint<32>  res_int_bits = *(reinterpret_cast<ap_uint<32>*>(&res_int));
	res_int_bits(30, 23) = res_int_bits(30, 23) - 28;
	res_int_bits[31] = sign;
	float res = *(reinterpret_cast<float*>(&res_int_bits));
	// return with new sign
	return res;//get_with_new_sign((float) res, sign);
}


float icdf_linear_interpolated(InterpolationIndex index) {
	ap_uint<10> table_index = (index.exponential_segment, index.linear_segment);
	InterpolationCoefficients coeffs = coeff_lut[table_index];
	#pragma HLS data_pack variable=coeff_lut
	ap_ufixed<17,0> x = *(reinterpret_cast<ap_ufixed<17,0>*>(
			&index.interpolation_x));
	ap_ufixed<41,-4> prod = coeffs.coeff_1 * x;
	ap_ufixed<32,4> res_fixed = prod + coeffs.coeff_2;
	float res = fixed_to_float(res_fixed, index.sign);
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

	//#pragma HLS DATAFLOW
	//#pragma HLS PIPELINE II=1

	//for (int i = 0; i < 1; ++i) {
	//for (int i = 0; i < 100; ++i) {
		#pragma HLS PIPELINE II=1
		uint32_t input = uniform_rns.read();
		InterpolationIndex index = get_next_interpolation_input(input);
		float res_float = icdf_linear_interpolated(index);

		if (index.is_valid)
			gaussian_rns.write(res_float);
	//}

}

