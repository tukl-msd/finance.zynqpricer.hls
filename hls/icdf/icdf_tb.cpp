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

float golden_model(uint32_t a) {
	a &= 8388607;
	a |= (127 << 23);
	a |= (1 << 31);
	return *(reinterpret_cast<float*>(&a));
}

int main(int argc, char *argv[]) {
	init_genrand(0);
	hls::stream<uint32_t> uniform_rns;
	hls::stream<float> gaussian_rns;

	uint32_t arn[100];
	for (int i = 0; i < 100; ++i) {
		uint32_t rn = genrand_int32();
		arn[i] = rn;
		uniform_rns.write(rn);
	}

	icdf(uniform_rns, gaussian_rns);

	int i = 0;
	bool failed = false;
	while (gaussian_rns.size() > 0) {
		float cpu_res = golden_model(arn[i]);
		float acc_res = gaussian_rns.read();
		if (cpu_res != acc_res) {
			std::cout << arn[i] << " " <<  cpu_res << " != " << acc_res << std::endl;
			failed = true;
		}
		++i;
	}

	std::cout << "done." << std::endl;

	return failed;
}

