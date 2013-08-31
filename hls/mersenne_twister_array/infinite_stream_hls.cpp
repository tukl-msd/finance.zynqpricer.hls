//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "infinite_stream_hls.hpp"

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_shift_reg.h>

typedef ap_uint<10> uint10_t;

void infinite_stream(uint32_t seeds[624], hls::stream<uint32_t> &random_numbers) {
	#pragma HLS interface ap_memory port=seeds
	#pragma HLS resource core=RAM_1P metadata="-bus_bundle slv0" variable=seeds

	#pragma HLS interface ap_fifo port=random_numbers
	#pragma HLS resource core=AXI4Stream variable=random_numbers

	#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle slv0" variable=return

	/*
	//hls::stream<uint32_t> fifo_227;
	uint32_t index_minus_0;

	//static uint32_t shiftreg[624];
	static ap_shift_reg<uint32_t, 624> reg;
	uint32_t next;



	for (unsigned cnt = 0; cnt < 1000; ++cnt) {

		#pragma HLS PIPELINE II=1

		if (cnt < 624)
			index_minus_0 = seeds[cnt];
		else
			index_minus_0 = next; //shiftreg[623];
		//	index_minus_0 = fifo_227.read();
		random_numbers.write(index_minus_0);

		//next = shiftreg[622];
		//for (int N = 623 - 1; N >= 0; --N) {
		//	if (N > 0)
		//		shiftreg[N] = shiftreg[N-1];
		//	else
		//		shiftreg[N] = index_minus_0;
		//}

		next = reg.shift(index_minus_0, 622);

		//fifo_227.write(index_minus_0);


	}


	for (unsigned cnt = 0; cnt < 624; ++cnt) {
//		fifo_227.read();
	}
	*/


	static ap_shift_reg<uint32_t, 225> sr_227;
	//hls::stream<uint32_t> fifo_227;
	static ap_shift_reg<uint32_t, 395> sr_396;
	//hls::stream<uint32_t> fifo_396;

	uint32_t index_minus_0, index_minus_396, index_minus_397;
	bool index_minus_396_valid = false;
	bool index_minus_397_valid = false;
	uint10_t cnt = 0;

	uint32_t result_last;
	bool result_last_valid = false;

	uint32_t sr_227_out;
	uint32_t sr_396_out;

	//for (unsigned i = 0; i < 1000; ++i) {
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
				if (result_last_valid)
					sr_227_out = sr_227.shift(result_last);
					//fifo_227.write(result_last);
				result_last = result;
				result_last_valid = true;
			}
		}

		//fifo_396.write(index_minus_0);
		sr_396_out = sr_396.shift(index_minus_0);

		if (cnt < 624)
			++cnt;
	}
}

