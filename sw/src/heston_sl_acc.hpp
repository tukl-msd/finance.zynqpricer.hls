//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#ifndef __HESTON_SL_ACC_HPP
#define __HESTON_SL_ACC_HPP

#include <stdint.h>

float heston_sl_hw(
		// call option
		float spot_price,
		float reversion_rate,
		float long_term_avg_vola,
		float vol_of_vol,
		float riskless_rate,
		float vola_0,
		float correlation,
		float time_to_maturity,
		float strike_price,
		// both knowckout
		float lower_barrier_value,
		float upper_barrier_value,
		// simulation params
		uint32_t step_cnt,
		uint32_t path_cnt);

#endif

