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
#include <algorithm>

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


/**
 * Accumulates all log prices from all streams and calculates
 * all the multi-level metrics on the fly
 */
template<typename calc_t>
class Pricer {
public:
	Pricer(const bool do_multilevel, const HestonParams params)
		: do_multilevel(do_multilevel), params(params),
			price_mean(0), price_variance(0), price_cnt(0) {
	}

	void handle_path(calc_t fine_path, calc_t coarse_path=0) {
		calc_t val = get_payoff(fine_path) - 
				(do_multilevel ? get_payoff(coarse_path) : (calc_t) 0);
		update_online_statistics(val);
	}

	Statistics get_statistics() {
		Statistics stats;
		stats.mean = price_mean;
		stats.variance =price_variance / (price_cnt - 1);
		stats.cnt = price_cnt;
		return stats;
	}
private:
	calc_t get_payoff(calc_t path) {
		return std::max((calc_t) 0, std::exp(path) - 
				(calc_t) params.strike_price);
	}

	void update_online_statistics(double val) {
		// See Knuth TAOCP vol 2, 3rd edition, page 232
		++price_cnt;
		double delta = val - price_mean;
		price_mean += delta / price_cnt;
		price_variance += delta * (val - price_mean);
	}

	const bool do_multilevel;
	const HestonParams params;

	double price_mean;
	double price_variance;
	uint32_t price_cnt;
};


#endif

