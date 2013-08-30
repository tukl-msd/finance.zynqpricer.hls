//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "infinite_stream_hls.hpp"

#include <iostream>

/*
int main(int argc, char *argv[]) {

	hls::stream<uint32_t> out;
	uint32_t seeds[624];

	for (unsigned i = 0; i < 624; ++i)
		seeds[i] = i;

	infinite_stream(seeds, out);

	if (out.size() != 1000) {
		std::cout << "Error out.size()" << std::endl;
		exit(1);
	}

	for (unsigned i = 0; i < 1000; ++i) {
		uint32_t hw = out.read();
		uint32_t sw = seeds[i % 624];
		if (hw != sw) {
			std::cout << "Data error at i = " << i << ": " << hw << " != " << sw << std::endl;
			exit(1);
		}
	}

	std::cout << "done." << std::endl;
}*/


#include <stdint.h>
#include <iostream>

#include "mt19937ar.h"

int main(int argc, char *argv[]) {

	// generate seeds
	uint32_t seeds[624];
	init_genrand(0);
	generate_numbers();
	get_mt_state(seeds);

	// get hardware random numbers
	hls::stream<uint32_t> random_numbers;
	infinite_stream(seeds, random_numbers);

	// compare with software model
	for (int i = 0; i < 1000 - 396 - 1; ++i) {
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

