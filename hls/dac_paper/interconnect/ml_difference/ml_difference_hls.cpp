//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 24. November 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <hls_stream.h>


void ml_difference(hls::stream<float> &s_in,
			hls::stream<float> &s_out) {
	#pragma HLS interface ap_fifo port=s_in
	#pragma HLS RESOURCE variable=s_in core=AXIS
	#pragma HLS interface ap_fifo port=s_out
	#pragma HLS RESOURCE variable=s_out core=AXIS
	#pragma HLS interface ap_ctrl_none port=return

	#pragma HLS PIPELINE II=2

	s_out.write(s_in.read() - s_in.read());

}
