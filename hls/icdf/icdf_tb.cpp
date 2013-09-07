//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 04. September 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "icdf_hls.hpp"

#include <iostream>
#include <stdint.h>
#include <iostream>

#include "mt19937ar.h"


int main(int argc, char *argv[]) {
	init_genrand(0);
	hls::stream<uint32_t> uniform_rns;
	hls::stream<float> gaussian_rns;

	const int N = 1000;

	uint32_t arn[N];
	for (int i = 0; i < N; ++i) {
		uint32_t rn = genrand_int32();
		arn[i] = rn;
		uniform_rns.write(rn);
	}

	for (int i = 0; i < N; ++i)
		icdf(uniform_rns, gaussian_rns);

	int i = 0;
	while (gaussian_rns.size() > 0) {
		float acc_res = gaussian_rns.read();
		std::cout << i << ": " <<  acc_res << std::endl;
		++i;
	}

	std::cout << "done." << std::endl;

	return 0;
}

