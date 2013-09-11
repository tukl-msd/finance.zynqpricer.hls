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
#include <iomanip>

// address of Random Number Generator
const unsigned AXI_HESTON_BASE_ADDR = 0x43C10000;
const unsigned AXI_HESTON_SIZE      = 0x00001000;

int main(int argc, char *argv[]) {
	// get device pointers
	IODev axi_heston(AXI_HESTON_BASE_ADDR, AXI_HESTON_SIZE);
	volatile unsigned &heston_ctrl = 
			*((volatile unsigned*)axi_heston.get_dev_ptr(0x00));
	bool heston_idle = 
			*(volatile unsigned*)(axi_heston.get_dev_ptr(0x00)) & 0x4;

	// get full address space
	const unsigned word_cnt = AXI_HESTON_SIZE / sizeof(uint32_t);
	uint32_t state[word_cnt];

	// read state to hardware rng
	for (unsigned i = 0; i < word_cnt; ++i) {
		state[i] = *((volatile uint32_t*)axi_heston.get_dev_ptr(0) + i);
	}

	// print state
	std::cout << "offset - uint32_t - float" << std::endl;
	for (int i = 0; i < word_cnt; ++i) {
		std::cout << std::hex << std::setw(8) << i * sizeof(uint32_t) << 
				":  " << std::setw(8) << state[i]
				<< "   " << reinterpret_cast<float*>(state)[i] << std::endl;
	}
	std::cout << "offset - uint32_t - float" << std::endl;

	// print status information
	std::cout << "Current heston_ctrl: " << std::hex << 
			heston_ctrl << std::endl;
	if (heston_idle)
		std::cout << "Heston is idle." << std::endl;
	else
		std::cout << "Heston running." << std::endl;
	return 0;
}

