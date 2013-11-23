//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 11. November 2013
//

#include "json_helper.hpp"

#include "json/json.h"

#include <iostream>

#include "observer.hpp"

void Observer::update_fpga_config(std::string path) {
	send("fpga_config", path);
}

unsigned Observer::register_accelerator(const std::string &instance) {
	ObserverInstanceStats stat;
	stats.push_back(stat);
	names.push_back(instance);
	return names.size() - 1;
}

void Observer::clear_accelerators() {
	stats.clear();
	names.clear();
}

void Observer::setup_sl(unsigned index, HestonParamsSL sl_params, 
		bool use_last_params) {
	if (is_enabled) {
		stats[index] = {sl_params.path_cnt, 0, false, 1, 
				std::chrono::steady_clock::now()};
		Json::Value json;
		if (!use_last_params) {
			json = dump_sl_params(sl_params);
		}
		send_from("setup_sl", names[index], json);
	}
}

void Observer::setup_ml(unsigned index, HestonParamsML ml_params,
		uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel,
		bool use_last_params) {
	if (is_enabled) {
		uint32_t scaling = (do_multilevel) ? 2 : 1;
		stats[index] = {path_cnt * scaling, 0, false, 
				scaling, std::chrono::steady_clock::now()};
		Json::Value json;
		if (true) {
			if (!use_last_params) {
				json["ml_params"] = dump_ml_params(ml_params);
			}
			json["step_cnt_fine"] = step_cnt_fine;
			json["path_cnt"] = path_cnt;
			json["do_multilevel"] = do_multilevel;
		}
		send_from("setup_ml", names[index], json);
	}
}

/* optimized for speed, as it is called in tight inner loops */
void Observer::register_new_path(unsigned index) {
	uint64_t path_done = ++stats[index].path_done;
	if (!(path_done & 0xff) && is_enabled) {
		process_path_done(index);
	}
}

/* slow processing of new path stats */
void Observer::process_path_done(unsigned index) {
	ObserverInstanceStats &stat = stats[index];
	stat.is_done = stat.path_done >= stat.path_cnt;
	std::chrono::duration<float> duration = 
			std::chrono::steady_clock::now() - stat.last_printing;
	if (stat.is_done || (duration.count() > print_wait_duration)) {
		uint64_t real_path_done = stat.path_done / stat.scaling;
		stat.last_printing = std::chrono::steady_clock::now();
		send_from("new_path", names[index], real_path_done);
	}
}

void Observer::all_paths_done(unsigned index) {
	stats[index].path_done = stats[index].path_cnt;
	process_path_done(index);
}

void Observer::enable(bool enabled) {
	is_enabled = enabled;
}

bool Observer::get_enabled() {
	return is_enabled;
}

void Observer::send(std::string event, Json::Value value) {
	if (is_enabled) {
		Json::Value root;
		root["evt"] = event;
		root["val"] = value;
		send_raw(root);
	}
}

void Observer::send_from(std::string event, std::string instance, 
		Json::Value value) {
	if (is_enabled) {
		Json::Value root;
		root["evt"] = event;
		root["inst"] = instance;
		root["val"] = value;
		send_raw(root);
	}
}

void Observer::send_raw(Json::Value root) {
	if (is_enabled) {
		root["cls"] = "observer";
		Json::FastWriter w;
		std::cout << w.write(root);
	}
}

