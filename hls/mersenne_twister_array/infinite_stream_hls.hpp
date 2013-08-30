//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <hls_stream.h>

#include <ap_int.h>

void infinite_stream(uint32_t seeds[624], hls::stream<uint32_t> &random_numbers);
