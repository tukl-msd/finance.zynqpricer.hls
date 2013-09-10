//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"
#include "json_helper.hpp"
#include "heston_both_acc.hpp"

#include "json/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cmath>
#include <fstream>


template <typename T>
class read_iterator {
public:
	read_iterator(const std::vector<Json::Value> fifos, const unsigned cnt, 
			const useconds_t sleep_usec=50) 
		: total_read_cnt(cnt), 
		  sleep_usec(sleep_usec),
		  device_cnt(fifos.size())
	{
		if (sizeof(T) != 4) {
			std::cerr << "ERROR: only sizeof(T) = 4 supported" 
					<< std::endl;
			exit(-1);
		}
		int i = 0;
		for (auto fifo: fifos) {
			devices.push_back(IODev(asHex(fifo["Offset Address"]), 
						asHex(fifo["Range"])));
		}
	}

	bool next(T &out) {
		if (words_read < total_read_cnt) {
			while (avail == 0) {
				// go to next device
				current_device = (current_device + 1) % device_cnt;
				avail = get_occupancy(devices[current_device]);
				if (avail == 0) {
					usleep(sleep_usec);
				}
			}
			--avail;
			++words_read;
			out = read_data(devices[current_device]);
			return true;
		} else {
			return false;
		}
	}

private:
	// get number of words available to read for fifo device
	unsigned get_occupancy(IODev &dev) {
		// Receive Data FIFO Occupancy (RDFO)
		unsigned rdfo = *((unsigned*)dev.get_dev_ptr(0x1C));
		return rdfo & 0x7fffffff;
	}

	// read next word from fifo device
	// Warning: reading from a device when no data is available 
	//          can lead to data corruption or even deadlock
	T read_data(IODev &dev) {
		// Receive Data FIFO Data (RDFD)
		return *((T*)dev.get_dev_ptr(0x20));
	}

private:
	std::vector<IODev> devices;
	unsigned total_read_cnt;
	unsigned device_cnt;
	useconds_t sleep_usec;

	// current device
	unsigned current_device = 0;
	// how many words are available for current device
	unsigned avail = 0;
	// number of words read
	unsigned words_read = 0;
};


struct HestonParamsHW {
	// call option
	float log_spot_price;
	float reversion_rate_TIMES_step_size_fine;
	float reversion_rate_TIMES_step_size_coarse;
	float long_term_avg_vola;
	float vol_of_vol_TIMES_sqrt_step_size_fine;
	float vol_of_vol_TIMES_sqrt_step_size_coarse;
	float double_riskless_rate; // = 2 * riskless_rate
	float vola_0;
	float correlation;
	// both knockout
	float log_lower_barrier_value;
	float log_upper_barrier_value;
	// simulation params
	uint32_t step_cnt_coarse;
	uint32_t ml_constant; // only 5 bit
	uint32_t do_multilevel; // boolean
	float half_step_size_fine; // = step_size_fine / 2
	float half_step_size_coarse; // = step_size_coarse / 2
	float sqrt_step_size_fine; // = sqrt(step_size_fine)
	float sqrt_step_size_coarse; // = sqrt(step_size_coarse)
	// = BARRIER_HIT_CORRECTION * sqrt_step_size
	float barrier_correction_factor_fine;
	float barrier_correction_factor_coarse;
	uint32_t path_cnt;
};


float heston_ml_hw_kernel(Json::Value bitstream, HestonParamsML ml_params, 
		uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel) {
	HestonParamsML p = ml_params;
	if (step_cnt_fine % p.ml_constant != 0) {
		std::cerr << "ERROR: step_cnt_fine % ml_constant != 0" << std::endl;
		exit(1);
	}
	// define heston hw parameters
	// continuity correction, see Broadie, Glasserman, Kou (1997)
	float barrier_hit_correction = 0.5826;
	uint32_t step_cnt_coarse = step_cnt_fine / p.ml_constant;
	float step_size_fine = p.time_to_maturity / step_cnt_fine;
	float step_size_coarse = p.time_to_maturity / step_cnt_coarse;
	float sqrt_step_size_fine = std::sqrt(step_size_fine);
	float sqrt_step_size_coarse = std::sqrt(step_size_coarse);
	HestonParamsHW params_hw = {
		(float) std::log(p.spot_price),
		(float) p.reversion_rate * step_size_fine,
		(float) p.reversion_rate * step_size_coarse,
		(float) p.long_term_avg_vola,
		(float) p.vol_of_vol * sqrt_step_size_fine,
		(float) p.vol_of_vol * sqrt_step_size_coarse,
		(float) (2 * p.riskless_rate),
		(float) p.vola_0,
		(float) p.correlation,
		(float) std::log(p.lower_barrier_value),
		(float) std::log(p.upper_barrier_value),
		step_cnt_coarse,
		p.ml_constant,
		do_multilevel,
		step_size_fine / 2,
		step_size_coarse / 2,
		sqrt_step_size_fine,
		sqrt_step_size_coarse,
		(float) barrier_hit_correction * sqrt_step_size_fine,
		(float) barrier_hit_correction * sqrt_step_size_coarse,
		0};

	// find accelerators
	std::vector<Json::Value> accelerators;
	std::vector<Json::Value> fifos;
	for (auto component: bitstream)
		if (component["__class__"] == "heston_ml") {
			accelerators.push_back(component);
			fifos.push_back(component["axi_fifo"]);
		}

	// start accelerators
	uint32_t acc_path_cnt = path_cnt / accelerators.size();
	int path_cnt_remainder = path_cnt % accelerators.size();
	for (auto acc: accelerators) {
		params_hw.path_cnt = acc_path_cnt + (path_cnt_remainder-- > 0 ? 1 : 0);
		start_heston_accelerator(acc, &params_hw, sizeof(params_hw));
	}

	// setup read iterator
	read_iterator<float> read_it(fifos, path_cnt);

	// calculate result
	double result = 0;
	float price;
	while (read_it.next(price)) {
		std::cout << price << std::endl;
		//result += std::max(0.f, std::exp(price) - (float) p.strike_price);
	}
	result *= std::exp(-p.riskless_rate * p.time_to_maturity) / path_cnt;
	return result;
}


float heston_ml_hw(Json::Value bitstream, HestonParamsML ml_params) {
	return heston_ml_hw_kernel(bitstream, ml_params, 16, 10, false);
}

