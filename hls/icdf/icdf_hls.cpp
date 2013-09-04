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


// return the number of leading zeros of input
ap_uint<4> count_leading_zeros(ap_uint<10> input) {
	bool inc_lz = true;
	ap_uint<4> lz = 0;
	// go through bits from front to back
	for (int i = 1; i <= 10; ++i) {
		// increment as long as there are zeros
		inc_lz &= (input & (1 << (10 - i))) == 0;
		if (inc_lz)
			lz = i;
	}
	return lz;
}


struct interpolation_index {
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
interpolation_index get_next_interpolation_index(
		hls::stream<uint32_t> &stream_in) {
	ap_uint<6> total_exp_segment = 0;
	while (true) {
		#pragma HLS PIPELINE II=1
		uint32_t input = stream_in.read();

		interpolation_index index;
		index.sign = input & (1 << 31);
		ap_uint<10> exp_bits = (input >> 21) & ((1 << 10) - 1);
		index.linear_segment = input & (((1 << 4) - 1) << 17);
		index.interpolation_x = input & (((1 << 17) - 1));

		ap_uint<4> curr_exp_segment = count_leading_zeros(exp_bits);
		total_exp_segment += curr_exp_segment;
		index.exponential_segment = total_exp_segment;

		if ((curr_exp_segment != 10) || (total_exp_segment == 60)) {
			return index;
		}
	}
}


void icdf(
		hls::stream<uint32_t> &uniform_rns,
		hls::stream<float> &gaussian_rns) {
	#pragma HLS interface ap_fifo port=uniform_rns
	#pragma HLS resource core=AXI4Stream variable=uniform_rns
	#pragma HLS interface ap_fifo port=gaussian_rns
	#pragma HLS resource core=AXI4Stream variable=gaussian_rns
	#pragma HLS interface ap_ctrl_none port=return

//	uint8_t leading_zeros = 0;
	for (int i = 0; i < 100; ++i) {


		//uint32_t input = uniform_rns.read();

		uint32_t unif_float = 0;
		interpolation_index index = get_next_interpolation_index(uniform_rns);
		unif_float |= index.exponential_segment;


		// cast to float
		float res_float = *(reinterpret_cast<float*>(&unif_float));

		// output when we found one leading zero
//		if ((exp_bits != 0) || (leading_zeros >= 128 )) {
			gaussian_rns.write(res_float);
//			leading_zeros = 0;
//		}
	}

}

