//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "mersenne_twister_array_hls.hpp"

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_shift_reg.h>

#define SIZE_AR_396 395
#define SIZE_AR_227 225

void mersenne_twister(uint32_t seeds[624], hls::stream<uint32_t> &random_numbers) {
	#pragma HLS interface ap_memory port=seeds
	#pragma HLS resource core=RAM_1P metadata="-bus_bundle slv0" variable=seeds

	#pragma HLS interface ap_fifo port=random_numbers
	#pragma HLS resource core=AXI4Stream variable=random_numbers

	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle slv0" variable=return

	static uint32_t ar_227[SIZE_AR_227];
	static ap_uint<8> cn_227 = 0;
	#pragma HLS dependence variable=ar_227 false
	static uint32_t ar_396[SIZE_AR_396];
	static ap_uint<9> cn_396 = 0;
	#pragma HLS dependence variable=ar_396 false

	uint32_t index_minus_0, index_minus_396, index_minus_397;
	static bool index_minus_396_valid = false;
	static bool index_minus_397_valid = false;
	static ap_uint<10> cnt = 0;

	uint32_t result_last;
	static bool result_last_valid = false;

	uint32_t sr_227_out;
	uint32_t sr_396_out;

	//for (unsigned i = 0; i < 1000; ++i) {
	//TODO(brugger): check if #pragma HLS PIPELINE II=1 rewind can be used 
	//    here instead of while(true) loop (officially not 
	//    supported)
	while (true) {

		#pragma HLS PIPELINE II=1

		if (cnt < 624)
			index_minus_0 = seeds[cnt];
		else
			index_minus_0 = sr_227_out;

		if (cnt >= 396)  {
			index_minus_397 = index_minus_396;
			index_minus_397_valid = index_minus_396_valid;

			index_minus_396 = sr_396_out;
			index_minus_396_valid = true;

			if (index_minus_397_valid) {
				uint32_t y0 = index_minus_397;

				// tempering

				uint32_t y1 = y0 ^ (y0 >> 11);
				uint32_t y2 = y1 ^ ((y1 << 7) & 0x9D2C5680);
				uint32_t y3 = y2 ^ ((y2 << 15) & 0xEFC60000);
				uint32_t y4 = y3 ^ (y3 >> 18);

				random_numbers.write(y4);


				// update state

				uint32_t index_0 = index_minus_397;
				uint32_t index_1 = index_minus_396;
				uint32_t index_m = index_minus_0;


				uint32_t mixbits = (index_0 & 0x80000000) | (index_1 & 0x7FFFFFFF);
				uint32_t twist = ((index_1 & 1) ? 0x9908B0DF : 0) ^ (mixbits >> 1);
				uint32_t result = index_m ^ twist;

				// insert register to better meet timing
				if (result_last_valid) {
					sr_227_out = ar_227[cn_227];
					ar_227[cn_227] = result_last;
					cn_227 = (cn_227 < SIZE_AR_227 - 1) ? cn_227 + 1 : 0;
				}
				result_last = result;
				result_last_valid = true;
			}
		}

		sr_396_out = ar_396[cn_396];
		ar_396[cn_396] = index_minus_0;
		cn_396 = (cn_396 < SIZE_AR_396 - 1) ? cn_396 + 1 : 0;

		if (cnt < 624)
			++cnt;
	}
}

