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

void to_exp_path(hls::stream<float> in, hls::stream<float> out, float K) {
	#pragma HLS interface ap_fifo port=in
	#pragma HLS resource core=AXI4Stream variable=in
	#pragma HLS interface ap_fifo port=out
	#pragma HLS resource core=AXI4Stream variable=out
	#pragma HLS interface ap_ctrl_none port=return

	#pragma HLS PIPELINE II=1

	float res = in.read();

	res = hls::expf(res);

	out.write(res);
}
