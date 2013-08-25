//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"

#include <iostream>
#include <cstring>

// address of Random Number Generator
const int AXI_RNG_BASE_ADDR    = 0x43C00000;

int main(int argc, char *argv[]) {
	// get device pointers
	IODev axi_rng(AXI_RNG_BASE_ADDR);
	volatile unsigned &rng_ctrl = *((unsigned*)axi_rng.get_dev_ptr(0x00));
	bool rng_idle = *(unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;

	// get full Mersenne Twister state
	uint32_t mt_state[624];

	// send state to hardware rng
	memcpy(mt_state, axi_rng.get_dev_ptr(0x1000), sizeof(mt_state));

	// print rng state
	for (int i = 0; i < 624; ++i) {
		std::cout << std::hex << i << " \t- " << mt_state[i] << std::endl;
	}

	// print status information
	std::cout << "Current rng_ctrl: " << std::hex << rng_ctrl << std::endl;
	if (rng_idle)
		std::cout << "Random number generator is idle." << std::endl;
	else
		std::cout << "Random number generator running." << std::endl;

	return 0;
}

