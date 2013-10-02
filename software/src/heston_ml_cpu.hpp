//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#ifndef __HESTON_ML_CPU_HPP__
#define __HESTON_ML_CPU_HPP__

#include "heston_common.hpp"
#include "heston_ml_both.hpp"
#include "heston_both_cpu.hpp"

template<typename calc_t>
calc_t heston_ml_cpu(const HestonParamsML p) {
	return heston_ml_control(p, heston_cpu_kernel<calc_t>);
}

#endif
