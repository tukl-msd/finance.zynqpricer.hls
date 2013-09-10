//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 26. August 2013
//

#include "iodev.hpp"

#include "mt19937/mt19937ar.h"

#include <iostream>
#include <cstring>

// address of Random Number Generator
const unsigned AXI_RNG_BASE_ADDR = 0x43C00000;
const unsigned AXI_RNG_SIZE      = 0x00002000;

int main(int argc, char *argv[]) {
	// get device pointers
	IODev axi_rng(AXI_RNG_BASE_ADDR, AXI_RNG_SIZE);
	volatile unsigned &rng_ctrl = 
			*((volatile unsigned*)axi_rng.get_dev_ptr(0x00));
	bool rng_idle = *(volatile unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;

	// get full Mersenne Twister state
	uint32_t mt_state[624];

	// send state to hardware rng
	for (unsigned i = 0; i < 624; ++i) {
		*((volatile uint32_t*)axi_rng.get_dev_ptr(0x1000) + i) = mt_state[i];
	}

	// get expected Mersenne Twister state
	uint32_t mt_state_expected[624];
	init_genrand(0);
	generate_numbers();
	get_mt_state(mt_state_expected);

	// print rng state
	std::cout << "index \t- expected \t- from hardware" << std::endl;
	for (int i = 0; i < 624; ++i) {
		std::cout << std::hex << i << " \t- " << mt_state_expected[i] 
				<< " \t- " << mt_state[i] << std::endl;
	}
	std::cout << "index \t- expected \t- from hardware" << std::endl;

	// print status information
	std::cout << "Current rng_ctrl: " << std::hex << rng_ctrl << std::endl;
	if (rng_idle)
		std::cout << "Random number generator is idle." << std::endl;
	else
		std::cout << "Random number generator running." << std::endl;
	return 0;
}

