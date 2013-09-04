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

//const uint32_t EXP_OFFSET 			= 127;

const uint32_t SIGN_BIT 			= 0x80000000; 	// = 1 << 31
const uint32_t MANTISSA_BITS 		= 0x7fffff; 	// = 2**23 - 1
const uint32_t EXPONENT_BITS 		= 0x7f800000; 	// = (2**8 - 1) << 23
const uint32_t EXPONENT_BIT_OFFSET 	= 23;


// count leading zeros of input
uint8_t count_leading_zeros(uint8_t input) {
	uint8_t inc_lz = true;
	uint8_t lz = 0;
	for (int i = 0; i <= 7; ++i) {
		inc_lz &= (input & (1 << (7 - i)) == 0);
		if (inc_lz)
			++lz;
	}
	return lz;
}


void icdf(
		hls::stream<uint32_t> &uniform_rns,
		hls::stream<float> &gaussian_rns) {
	#pragma HLS interface ap_fifo port=uniform_rns
	#pragma HLS resource core=AXI4Stream variable=uniform_rns
	#pragma HLS interface ap_fifo port=gaussian_rns
	#pragma HLS resource core=AXI4Stream variable=gaussian_rns
	#pragma HLS interface ap_ctrl_none port=return

	uint8_t leading_zeros = 0;
	for (int i = 0; i < 100; ++i) {
		#pragma HLS PIPELINE II=1

		uint32_t input = uniform_rns.read();
		// resulting floating point as bit interpretation
		uint32_t unif_float = 0;

		// copy sign from input
		unif_float |= input & SIGN_BIT;
		// copy mantissa from input
		unif_float |= input & MANTISSA_BITS;

		// exponent = leading_zeros
		uint8_t exp_bits = (input & EXPONENT_BITS) >> EXPONENT_BIT_OFFSET;
		leading_zeros += count_leading_zeros(exp_bits);
		unif_float |= leading_zeros << EXPONENT_BIT_OFFSET;

		// cast to float
		float res_float = *(reinterpret_cast<float*>(&unif_float));

		// output when we found one leading zero
		if ((exp_bits != 0) || (leading_zeros >= 128 )) {
			gaussian_rns.write(res_float);
			leading_zeros = 0;
		}
	}

}

