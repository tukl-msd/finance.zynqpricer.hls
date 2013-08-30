//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "antithetic_hls.hpp"

void antithetic(
		hls::stream<float> &rn_in,
		hls::stream<float> &rn_out_1,
		hls::stream<float> &rn_out_2)
{
	#pragma HLS interface ap_fifo port=rn_in
	#pragma HLS resource core=AXI4Stream variable=rn_in

	#pragma HLS interface ap_fifo port=rn_out_1
	#pragma HLS resource core=AXI4Stream variable=rn_out_1
	#pragma HLS interface ap_fifo port=rn_out_2
	#pragma HLS resource core=AXI4Stream variable=rn_out_2

	#pragma HLS interface ap_ctrl_none port=return

	//for (int i = 0; i < 10 / 2; ++i) {
	while (true) {
		#pragma HLS PIPELINE II=2

		float r1 = rn_in.read();
		float r2 = rn_in.read();

		rn_out_1.write(r1);
		rn_out_2.write(r2);

		rn_out_1.write(-r1);
		rn_out_2.write(-r2);
	}
}
