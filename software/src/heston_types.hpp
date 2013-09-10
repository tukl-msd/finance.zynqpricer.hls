//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 10. September 2013
//

#ifndef __HESTON_TYPES_HPP__
#define __HESTON_TYPES_HPP__

#include <stdint.h>

struct HestonParams {
	// call option
	double spot_price;
	double reversion_rate;
	double long_term_avg_vola;
	double vol_of_vol;
	double riskless_rate;
	double vola_0;
	double correlation;
	double time_to_maturity;
	double strike_price;
	// both knowckout
	double lower_barrier_value;
	double upper_barrier_value;
};

struct HestonParamsSL : HestonParams {
	uint32_t step_cnt;
	uint64_t path_cnt;
};

struct HestonParamsML : HestonParams {
	uint32_t ml_l_start;
	uint32_t ml_constant;
	uint32_t ml_path_cnt_start;
	double ml_epsilon;
};

#endif

