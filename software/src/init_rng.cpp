//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"
#include "json_helper.hpp"

#include "json/json.h"
#include "mt19937/mt19937ar.h"

#include <iostream>
#include <cstring>

// address of Random Number Generator
bool initialize_mersenne_twister(Json::Value mt, uint32_t seed){
	// get address
	unsigned base = asHex(mt["Offset Address"]);
	unsigned range = asHex(mt["Range"]);
	// get device pointers
	IODev axi_rng(base, range);
	volatile unsigned &rng_ctrl = 
			*((volatile unsigned*)axi_rng.get_dev_ptr(0x00));
	bool rng_idle = *(volatile unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;

	// exit if rng is already running
	if (!rng_idle) {
		std::cerr << "WARNING: Random number generator at address " <<
				std::hex << base << " is already running." << std::endl;
		return false;
	}
	
	// get full Mersenne Twister state
	uint32_t mt_state[624];
	init_genrand(seed);
	generate_numbers();
	get_mt_state(mt_state);

	// send state to hardware rng
	for (unsigned i = 0; i < 624; ++i) {
		*((volatile uint32_t*)axi_rng.get_dev_ptr(0x1000) + i) = mt_state[i];
	}
	
	// start hardware rng
	rng_ctrl = 1;

	std::cout << "Initialized Random Number Generator at address " << 
				std::hex << base << " with seed " << seed << "." << std::endl;
	return true;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " bitstream.json" << std::endl;
		return -1;
	}
	Json::Value bitstream = read_params(argv[1]);
	uint32_t cnt = 0;
	for (auto component: bitstream) {
		if (component["__class__"] == "heston_sl"
				|| component["__class__"] == "heston_ml") {
			initialize_mersenne_twister(
					component["mersenne_twister"],
					cnt /* = seed*/);
			++cnt;
		}
	}
	if (cnt == 0) {
		std::cout << "WARNING: no component found" << std::endl;
	}
}

