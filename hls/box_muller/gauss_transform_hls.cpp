//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 19. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "gauss_transform_hls.hpp"

#include <hls_math.h>

#include <ap_int.h>

void gauss_transform(
		hls::stream<uint32_t> &uniform_rns,
		hls::stream<float> &gaussian_rns) {
	#pragma HLS interface ap_fifo port=uniform_rns
	#pragma HLS resource core=AXI4Stream variable=uniform_rns

	#pragma HLS interface ap_fifo port=gaussian_rns
	#pragma HLS resource core=AXI4Stream variable=gaussian_rns

	#pragma HLS interface ap_ctrl_none port=return

	float u1, u2, r, z1, z2;
	while (true){
	//for (int i = 0; i < 100/2; ++i) {
		#pragma HLS PIPELINE II=2
		// intervall (0:1]
		u1 = ((float)uniform_rns.read() + 1.f) * (float)(1.0 / 4294967296.0);
		// intervall (0:2PI]
		u2 = ((float)uniform_rns.read() + 1.f) * (float)(2 * M_PI / 4294967296.0);
		r = hls::sqrtf(-2 * hls::logf(u1));
		z1 = r * hls::cosf(u2);
		z2 = r * hls::sinf(u2);
		gaussian_rns.write(z1);
		gaussian_rns.write(z2);
	}

}
