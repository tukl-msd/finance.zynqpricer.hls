//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#ifndef __HESTON_ML_BOTH_HPP__
#define __HESTON_ML_BOTH_HPP__

#include "heston_common.hpp"

#include <functional>


double heston_ml_control(const HestonParamsML &ml_params,
		std::function<Statistics(const HestonParamsML, const uint32_t, 
		const uint64_t, const bool, const uint32_t)> ml_kernel,
		bool do_print=true);

#endif
