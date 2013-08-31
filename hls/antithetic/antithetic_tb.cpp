//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 26. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "antithetic_hls.hpp"

#include <iostream>

int main() {
	hls::stream<float> in1;
	hls::stream<float> out1;
	hls::stream<float> out2;

	float in[10];

	for (int i = 0; i < 10; ++i) {
		in[i] = i * 1.25;
		in1.write(i * 1.25);
	}

	antithetic(in1, out1, out2);

	if (out1.size() != 10 or out2.size() != 10) {
		std::cout << "ERROR: wrong size" << std::endl;
	}
	for (int i = 0; i < 10; ++i) {
		float exp1 = (i % 2 ? -1 : 1) * in[i/2*2];
		float exp2 = (i % 2 ? -1 : 1) * in[i/2*2 + 1];
		float hw1 = out1.read();
		float hw2 = out2.read();
		if (exp1 != hw1 or exp2 != hw2) {
			std::cout << "ERROR: " << i << " \t" << hw1 <<
					" \t"  << hw2 << std::endl;
			return -1;
		}
	}
	std::cout << "passed." << std::endl;
	return 0;
}
