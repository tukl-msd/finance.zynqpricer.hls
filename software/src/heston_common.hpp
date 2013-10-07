//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 10. September 2013
//

#ifndef __HESTON_COMMON_HPP
#define __HESTON_COMMON_HPP

#include <stdint.h>
#include <iostream>

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
	uint32_t ml_start_level;
	uint32_t ml_constant;
	uint32_t ml_path_cnt_start;
	double ml_epsilon;
};

struct HestonParamsEval : HestonParams {
	uint32_t eval_start_level;
	uint32_t eval_stop_level;
	uint32_t ml_start_level;
	uint32_t ml_constant;
	uint32_t path_cnt;
};


struct Statistics {
	double mean;
	double variance;
	uint64_t cnt;

	Statistics();
	Statistics(double mean, double variance, uint64_t cnt);
	Statistics& operator+=(const Statistics &rhs);
	Statistics& operator*=(const double &rhs);
};

inline Statistics operator+(Statistics lhs, const Statistics &rhs) {
	lhs += rhs;
	return lhs;
}

inline Statistics operator*(Statistics lhs, const double &rhs) {
	lhs *= rhs;
	return lhs;
}

std::ostream& operator<<(std::ostream& o, const Statistics &s);



#endif

