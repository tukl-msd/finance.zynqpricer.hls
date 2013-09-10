//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 10. September 2013
//

#include "iodev.hpp"
#include "json_helper.hpp"

#include "json/json.h"

#include <stdint.h>

#include "heston_both_acc.hpp"

void start_heston_accelerator(const Json::Value heston, void *params, 
		unsigned params_size) {
	// get device pointers
	IODev axi_heston(asHex(heston["heston_kernel"]["Offset Address"]), 
			asHex(heston["heston_kernel"]["Range"]));
	volatile unsigned &acc_ctrl = *((unsigned*)axi_heston.get_dev_ptr(0x00));

	// make sure heston accelerator is not running
	bool acc_idle = acc_ctrl & 0x4;
	if (!acc_idle) {
		std::cerr << "Heston accelerator is already running." << std::endl;
		exit(EXIT_FAILURE);
	}

	// make sure random number generator is running
	IODev axi_rng(asHex(heston["mersenne_twister"]["Offset Address"]), 
			asHex(heston["mersenne_twister"]["Range"]));
	bool rng_idle = *(unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;
	if (rng_idle) {
		std::cerr << "Start the Random Number generator before running"
			<< " this application" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// send parameters to accelerator, 64 bit aligned
	for (unsigned i = 0x14, j = 0; j < params_size / 4; i = i + 8, ++j) {
		*((uint32_t*)axi_heston.get_dev_ptr(i)) = 
				*((uint32_t*)params + j);
	}

	// start heston accelerator
	acc_ctrl = 1;
}


