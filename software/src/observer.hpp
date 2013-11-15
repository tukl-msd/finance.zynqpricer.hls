//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 11. November 2013
//

#ifndef __OBSERVER_HPP
#define __OBSERVER_HPP

#include "heston_common.hpp"

#include "json/json-forwards.h"

#include <string>
#include <map>
#include <memory>
#include <chrono>

struct ObserverInstanceStats {
	uint64_t path_cnt;
	uint64_t path_done;
	bool is_done;
	// how man paths are registered for each real path
	uint32_t scaling;
	std::chrono::steady_clock::time_point last_printing;
};

// singleton Observer
class Observer {
private:
	Observer() : is_enabled(false) {};
	Observer(Observer const&); // Don't implement
	void operator=(Observer const&); // Don't implment
public:
	static Observer& getInstance() {
		static Observer inst;
		return inst;
	}

	void update_fpga_config(std::string path);

	unsigned register_accelerator(const std::string &instance);
	void clear_accelerators();

	void setup_sl(unsigned index, HestonParamsSL sl_params);
	void setup_ml(unsigned index, HestonParamsML ml_params,
			uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel);
	void register_new_path(unsigned index);
	void all_paths_done(unsigned index);

	void enable(bool enabled=true);
	bool get_enabled();
private:
	void process_path_done(unsigned index);
	void send(std::string event, Json::Value value);
	void send_from(std::string event, std::string instance, Json::Value value);
	void send_raw(Json::Value root);

	bool is_enabled;
	std::vector<ObserverInstanceStats> stats;
	std::vector<std::string> names;

	const double print_wait_duration = 0.1; // in seconds
};

#endif

