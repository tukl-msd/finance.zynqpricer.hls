//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 7. September 2013
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

void call(hls::stream<float> in, hls::stream<float> out, float K) {
	#pragma HLS interface ap_fifo port=in
	#pragma HLS resource core=AXI4Stream variable=in
	#pragma HLS interface ap_fifo port=out
	#pragma HLS resource core=AXI4Stream variable=out
	#pragma HLS interface ap_none port=K
	#pragma HLS resource core=AXI4LiteS variable=K
	#pragma HLS interface ap_ctrl_none port=return

	#pragma HLS PIPELINE II=1

	float res = in.read();

	res = max_0(res - K);

	out.write(res);
}
