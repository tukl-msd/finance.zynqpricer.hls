//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 19. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "mersenne_twister_hls.hpp"

#include <ap_int.h>

typedef ap_uint<10> uint10_t;

//TODO(brugger): get Co-simulation working, compiler bug with hls:stream?
void mersenne_twister(uint32_t seeds[624], hls::stream<uint32_t> &random_numbers) {
	#pragma HLS interface ap_memory port=seeds
	#pragma HLS resource core=RAM_1P metadata="-bus_bundle slv0" variable=seeds

	#pragma HLS interface ap_fifo port=random_numbers
	#pragma HLS resource core=AXI4Stream variable=random_numbers

	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle slv0" variable=return

	hls::stream<uint32_t> fifo_227;
	hls::stream<uint32_t> fifo_396;

	#pragma HLS stream depth=226 variable=fifo_227
	#pragma HLS stream depth=396 variable=fifo_396

	uint32_t index_minus_0, index_minus_396, index_minus_397;
	bool index_minus_396_valid = false;
	bool index_minus_397_valid = false;
	uint10_t cnt = 0;

	uint32_t result_last;
	bool result_last_valid = false;

#ifndef __SYNTHESIS__
	unsigned cnt_output = _SIM_RN_CNT_;
#endif

	//while (true) {
	for (int i = -396; i <= _SIM_RN_CNT_; ++i) {

		#pragma HLS PIPELINE II=1

		if (cnt < 624)
			index_minus_0 = seeds[cnt];
		else
			index_minus_0 = fifo_227.read();

		if (cnt >= 396)  {
			index_minus_397 = index_minus_396;
			index_minus_397_valid = index_minus_396_valid;

			index_minus_396 = fifo_396.read();
			index_minus_396_valid = true;

			if (index_minus_397_valid) {
				uint32_t y0 = index_minus_397;

				// tempering

				uint32_t y1 = y0 ^ (y0 >> 11);
				uint32_t y2 = y1 ^ ((y1 << 7) & 0x9D2C5680);
				uint32_t y3 = y2 ^ ((y2 << 15) & 0xEFC60000);
				uint32_t y4 = y3 ^ (y3 >> 18);

				random_numbers.write(y4);

#ifndef __SYNTHESIS__
				if (--cnt_output == 0)
					break;
#endif

				// update state

				uint32_t index_0 = index_minus_397;
				uint32_t index_1 = index_minus_396;
				uint32_t index_m = index_minus_0;


				uint32_t mixbits = (index_0 & 0x80000000) | (index_1 & 0x7FFFFFFF);
				uint32_t twist = ((index_1 & 1) ? 0x9908B0DF : 0) ^ (mixbits >> 1);
				uint32_t result = index_m ^ twist;

				// insert register to better meet timing
				if (result_last_valid)
					fifo_227.write(result_last);
				result_last = result;
				result_last_valid = true;
			}
		}

		fifo_396.write(index_minus_0);

		if (cnt < 624)
			++cnt;
	}
}
