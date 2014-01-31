//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 04. September 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#ifndef __ICDF_HLS_HPP_
#define __ICDF_HLS_HPP_

#include <hls_stream.h>
#include <ap_int.h>
#include <stdint.h>

void icdf(hls::stream<uint32_t> &uniform_rns, hls::stream<float> &gaussian_rns);

#endif
