//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 26. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#ifndef __ANTITHETIC_HLS_HPP__
#define __ANTITHETIC_HLS_HPP__

#include <hls_stream.h>

void antithetic(
		hls::stream<float> &rn_in,
		hls::stream<float> &rn_out_1,
		hls::stream<float> &rn_out_2);

#endif
