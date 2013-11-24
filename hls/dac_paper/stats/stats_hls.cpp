//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 23. November 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <hls_stream.h>
#include <hls_math.h>
#include <ap_fixed.h>

float max_0(float x) {
	ap_uint<32> x_bits = *reinterpret_cast<ap_uint<32>*>(&x);
	if (x_bits[31] == 1)
		return 0;
	else
		return x;
}

typedef double calc_t;

void pricing(hls::stream<float> in, hls::stream<calc_t> out,
		hls::stream<calc_t> out2, float strike_price) {

	const int BLOCK = 64;
	static calc_t res_sum[BLOCK];
	//#pragma HLS stream depth=64 variable=res_sum
	#pragma HLS dependence variable=res_sum false

	static calc_t res_prod_sum[BLOCK];
	//#pragma HLS stream depth=64 variable=res_prod_sum
	#pragma HLS dependence variable=res_prod_sum false

	//#pragma HLS PIPELINE II=BLOCK

	while (true) {
		for (int i = 0; i < BLOCK; ++i) {
			#pragma HLS PIPELINE II=2
			float path = in.read();
			//float res = max_0(hls::expf(path) - strike_price);
			float res = path;


			calc_t l_sum = res_sum[i];
			calc_t l_prod_sum = res_prod_sum[i];

			calc_t n_sum = l_sum + (calc_t) res;
			calc_t n_prod_sum = l_prod_sum + (calc_t) (res * res);

			res_sum[i] = n_sum;
			res_prod_sum[i] = n_prod_sum;

			out.write(n_sum);
			out2.write(n_prod_sum);
		}
	}
}
