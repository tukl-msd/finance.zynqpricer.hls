//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#ifndef __HESTON_SL_CPU_HPP__
#define __HESTON_SL_CPU_HPP__

#include <stdint.h>

typedef float calc_t;

calc_t heston_sl_cpu(
		// call option
		calc_t spot_price,
		calc_t reversion_rate,
		calc_t long_term_avg_vola,
		calc_t vol_of_vol,
		calc_t riskless_rate,
		calc_t vola_0,
		calc_t correlation,
		calc_t time_to_maturity,
		calc_t strike_price,
		// both knowckout
		calc_t lower_barrier_value,
		calc_t upper_barrier_value,
		// simulation params
		uint32_t step_cnt,
		uint32_t path_cnt);

#endif
