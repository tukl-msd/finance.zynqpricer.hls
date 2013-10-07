//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 29. August 2013
//

#include "json_helper.hpp"

#include "json/json.h"

#include <fstream>
#include <sstream>
#include <string>


Json::Value read_params(char *filename) {
	std::ifstream file(filename);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(file, root)) {
		std::cerr << "Failed to parse parameter file" << std::endl
				<< reader.getFormattedErrorMessages();
		exit(-1);
	}
	return root;
}

void parse_heston_params(Json::Value json, HestonParams &params) {
	// heston params
	auto heston = json["heston"];
	params.spot_price = heston["spot_price"].asDouble();
	params.reversion_rate = heston["reversion_rate"].asDouble();
	params.long_term_avg_vola = heston["long_term_avg_vola"].asDouble();
	params.vol_of_vol = heston["vol_of_vol"].asDouble();
	params.riskless_rate = heston["riskless_rate"].asDouble();
	params.vola_0 = heston["vola_0"].asDouble();
	params.correlation = heston["correlation"].asDouble();
	params.time_to_maturity = heston["time_to_maturity"].asDouble();
	params.strike_price = heston["strike_price"].asDouble();
	// both knowckout
	auto barrier = json["barrier values"];
	params.lower_barrier_value = barrier["lower"].asDouble();
	params.upper_barrier_value = barrier["upper"].asDouble();
}

HestonParamsSL get_sl_params(Json::Value json) {
	HestonParamsSL params;
	parse_heston_params(json, params);
	// simulation params
	auto simulation = json["simulation_sl"];
	params.step_cnt = simulation["step_cnt"].asUInt();
	params.path_cnt = simulation["path_cnt"].asUInt64();
	return params;
}


HestonParamsML get_ml_params(Json::Value json) {
	HestonParamsML params;
	parse_heston_params(json, params);
	// simulation params
	auto simulation = json["simulation_ml"];
	params.ml_start_level = simulation["start_level"].asUInt();
	params.ml_constant = simulation["ml_constant"].asUInt();
	params.ml_path_cnt_start = simulation["path_cnt_start"].asUInt();
	params.ml_epsilon = simulation["epsilon"].asDouble();
	return params;
}

HestonParamsEval get_eval_params(Json::Value json) {
	HestonParamsEval params;
	parse_heston_params(json, params);
	auto simulation = json["simulation_eval"];
	params.eval_start_level = simulation["start_level"].asUInt();
	params.eval_stop_level = simulation["stop_level"].asUInt();
	params.ml_start_level = simulation["ml_start_level"].asUInt();
	params.ml_constant = simulation["ml_constant"].asUInt();
	params.path_cnt = simulation["path_cnt"].asUInt();
	return params;
}

unsigned asHex(Json::Value val) {
	return std::stoul(val.asString(), 0, 0);
}


