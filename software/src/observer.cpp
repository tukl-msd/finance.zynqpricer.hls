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
		stats[instance] = {sl_params.path_cnt, 0};
		send_from("setup_sl", instance, dump_sl_params(sl_params));
	}
}

void Observer::setup_ml(const std::string &instance, HestonParamsML ml_params,
		uint32_t step_cnt_fine, uint32_t path_cnt, bool do_multilevel) {
	if (is_enabled) {
		stats[instance] = {path_cnt, 0};
		Json::Value json;
		json["ml_params"] = dump_ml_params(ml_params);
		json["step_cnt_fine"] = step_cnt_fine;
		json["path_cnt"] = path_cnt;
		json["do_multilevel"] = do_multilevel;
		send_from("setup_ml", instance, json);
	}
}

void Observer::register_new_path(const std::string &instance) {
	if (is_enabled) {
		uint64_t path_done = ++stats[instance].path_done;
		if (path_done % 1000 == 0 || path_done == stats[instance].path_cnt) {
			send_from("new_path", instance, path_done);
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

