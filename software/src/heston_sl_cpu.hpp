//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

// Optimizations:
// - pre-calculation
// - using best known gaussion transformation method: ziggurat
// - vectorizable loops (AVX)
// - exploit loop pipelining / blocking
//
// Compiler flags:
// - MSCV: /O2 /arch:AVX /fp:fast /GL
// - GCC on ARM: -O3 -march=native -ffast-math -mfpu=neon
// - GCC on Intel: -O3 -march=native -ffast-math

#ifndef __HESTON_SL_CPU_HPP__
#define __HESTON_SL_CPU_HPP__

#include "heston_common.hpp"
#include "heston_both_cpu.hpp"

template<typename calc_t>
calc_t heston_sl_cpu(HestonParamsSL p) {
	return (calc_t) heston_cpu_kernel_sl<calc_t>(
			p, p.step_cnt, p.path_cnt).mean;
}

#endif
