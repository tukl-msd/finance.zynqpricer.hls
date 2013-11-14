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

void Observer::setup_sl(const std::string &instance, HestonParamsSL sl_params) {
	if (is_enabled) {
		stats[instance] = {sl_params.path_cnt, 0, false, 1, 
				std::chrono::steady_clock::now()};
		send_from("setup_sl", instance, dump_sl_params(sl_params));
	}
}

void Observer::setup_ml(const std::string &instance, HestonParamsML ml_params,
		uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel) {
	if (is_enabled) {
		uint32_t scaling = (do_multilevel) ? 2 : 1;
		stats[instance] = {path_cnt * scaling, 0, false, 
				scaling, std::chrono::steady_clock::now()};
		Json::Value json;
		json["ml_params"] = dump_ml_params(ml_params);
		json["step_cnt_fine"] = step_cnt_fine;
		json["path_cnt"] = path_cnt;
		json["do_multilevel"] = do_multilevel;
		send_from("setup_ml", instance, json);
	}
}

void Observer::register_new_path(const std::string &instance) {
	if (is_enabled && !stats[instance].is_done) {
		ObserverInstanceStats &stat = stats[instance];
		++stat.path_done;
		stat.is_done = stat.path_done >= stat.path_cnt;
		std::chrono::duration<float> duration = 
				std::chrono::steady_clock::now() - stat.last_printing;
		if (stat.is_done || (duration.count() > print_wait_duration)) {
			uint64_t real_path_done = stat.path_done / stat.scaling;
			stat.last_printing = std::chrono::steady_clock::now();
			send_from("new_path", instance, real_path_done);
		}
	}
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
		root["__event__"] = event;
		root["__value__"] = value;
		send_raw(root);
	}
}

void Observer::send_from(std::string event, std::string instance, 
		Json::Value value) {
	if (is_enabled) {
		Json::Value root;
		root["__event__"] = event;
		root["__instance__"] = instance;
		root["__value__"] = value;
		send_raw(root);
	}
}

void Observer::send_raw(Json::Value root) {
	if (is_enabled) {
		root["__class__"] = "observer";
		Json::FastWriter w;
		std::cout << w.write(root);
	}
}

