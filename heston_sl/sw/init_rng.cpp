//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"

#include "mt19937ar.h"

#include <iostream>
#include <cstring>

// address of Random Number Generator
const int AXI_RNG_BASE_ADDR    = 0x43C00000;

int main(int argc, char *argv[]) {
	// get device pointers
	IODev axi_rng(AXI_RNG_BASE_ADDR);
	volatile unsigned &rng_ctrl = *((unsigned*)axi_rng.get_dev_ptr(0x00));
	bool rng_idle = *(unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;

	// exit if rng is already running
	if (!rng_idle) {
		std::cerr << "Random number generator is already running."
				<< std::endl;
		exit(EXIT_FAILURE);
	}
	
	// get full Mersenne Twister state
	uint32_t mt_state[624];
	init_genrand(0);
	generate_numbers();
	get_mt_state(mt_state);

	// send state to hardware rng
	memcpy(axi_rng.get_dev_ptr(0x1000), mt_state, sizeof(mt_state));
	
	// start hardware rng
	rng_ctrl = 1;

	std::cout << "Random Number Generator has been started." << std::endl;

	return 0;
}

