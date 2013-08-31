//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 19. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <stdint.h>
#include <iostream>

#include "mt19937ar.h"

#include "mersenne_twister_hls.hpp"

int main(int argc, char *argv[]) {

	// generate seeds
	uint32_t seeds[624];
	init_genrand(0);
	generate_numbers();
	get_mt_state(seeds);

	// get hardware random numbers
	hls::stream<uint32_t> random_numbers;
	mersenne_twister(seeds, random_numbers);

	// compare with software model
	for (int i = 0; i < _SIM_RN_CNT_; ++i) {
		uint32_t rn_sw = genrand_int32();
		uint32_t rn_hw = random_numbers.read();
		if (rn_sw != rn_hw) {
			std::cout << i << ": " << rn_sw << " != " << rn_hw << std::endl;
			return -1;
		}
	}

	std::cout << "test passed." << std::endl;

	return 0;
}
