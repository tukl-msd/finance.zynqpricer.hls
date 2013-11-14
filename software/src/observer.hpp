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

struct ObserverInstanceStats {
	uint64_t path_cnt;
	uint64_t path_done;
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

	void setup_sl(const std::string &instance, HestonParamsSL sl_params);
	void setup_ml(const std::string &instance, HestonParamsML ml_params,
			uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel);
	void register_new_path(const std::string &instance);

	void enable(bool enabled=true);
	bool get_enabled();
private:
	void send(std::string event, Json::Value value);
	void send_from(std::string event, std::string instance, Json::Value value);
	void send_raw(Json::Value root);

	bool is_enabled;
	std::map<std::string, ObserverInstanceStats> stats;
};

#endif

