//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 26. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include "gauss_transform_hls.hpp"

#include <iostream>
#include <cmath>

#include "mt19937ar.h"

float to_float(uint32_t x) {
	return ((float)x + 1.0f) / 4294967296.0f;
}

void box_muller(float &u1, float &u2) {
	float r = std::sqrt(-2.0f * std::log(u1));
	float phi = 2.0f * 3.14159265358979f * u2;
	u1 = r * std::cos(phi);
	u2 = r * std::sin(phi);
}

int main() {
	init_genrand(0);

	uint32_t ain[100];
	hls::stream<uint32_t> din;
	hls::stream<float> dout;

	for (int i = 0; i < 100; ++i) {
		uint32_t rn = genrand_int32();;
		ain[i] = rn;
		din.write(rn);
	}

	float aout[100];
	for (int i = 0; i < 100/2; ++i) {
		float u1 = to_float(ain[2 * i]);
		float u2 = to_float(ain[2 * i + 1]);
		box_muller(u1, u2);
		aout[2 * i] = u1;
		aout[2 * i + 1] = u2;
	}

	gauss_transform(din, dout);

	if (dout.size() != 100) {
		std::cout << "wrong size" << std::endl;
		return -1;
	}

	for (int i = 0; i < 100; ++i) {
		float hw = dout.read();
		float sw = aout[i];
		std::cout << i << ": \t" << hw << " \t(hw) - (sw) " << sw <<
				std::endl;
		if (!(std::isfinite(hw) and std::isfinite(sw) and
				std::abs(hw - sw) < 1E-4)) {
			std::cout << "ERROR: " << i << ": \t" << hw << " \t(hw|sw) "
					<< sw << std::endl;
			return -1;
		}
	}

	std::cout << "passed." << std::endl;
	return 0;
}
