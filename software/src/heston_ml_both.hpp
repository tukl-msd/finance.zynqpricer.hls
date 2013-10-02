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

/**
 * Accumulates all log prices from all streams and calculates
 * all the multi-level metrics on the fly
 */
class Pricer {
public:
	Pricer(const bool do_multilevel, const HestonParams params);
	void handle_path(float fine_path, float coarse_path=0);
	Statistics get_statistics();
private:
	float get_payoff(float path);
	void update_online_statistics(float val);

	const bool do_multilevel;
	const HestonParams params;

	double price_mean;
	double price_variance;
	uint32_t price_cnt;
};

double heston_ml_control(const HestonParamsML &ml_params,
		std::function<Statistics(const HestonParamsML, const uint32_t, 
		const uint64_t, const bool, const uint32_t)> ml_kernel);

#endif
