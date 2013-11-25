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


Json::Value read_params(const char *filename) {
	std::ifstream file(filename);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(file, root)) {
		std::cerr << "Failed to parse parameter file: " << filename
				<< std::endl << reader.getFormattedErrorMessages();
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


Json::Value dump_heston_params(HestonParams params) {
	Json::Value heston;
	heston["spot_price"] = params.spot_price;
	heston["reversion_rate"] = params.reversion_rate;
	heston["long_term_avg_vola"] = params.long_term_avg_vola;
	heston["vol_of_vol"] = params.vol_of_vol;
	heston["riskless_rate"] = params.riskless_rate;
	heston["vola_0"] = params.vola_0;
	heston["correlation"] = params.correlation;
	heston["time_to_maturity"] = params.time_to_maturity;
	heston["strike_price"] = params.strike_price;
	// both knowckout
	Json::Value barrier;
	barrier["lower"] = params.lower_barrier_value;
	barrier["upper"] = params.upper_barrier_value;
	// build json data
	Json::Value json;
	json["heston"] = heston;
	json["barrier"] = barrier;
	return json;
}

Json::Value dump_sl_params(HestonParamsSL params) {
	auto json = dump_heston_params(params);
	// simulation params
	Json::Value simulation;
	simulation["step_cnt"] = params.step_cnt;
	simulation["path_cnt"] = params.path_cnt;
	json["simulation_sl"] = simulation;
	return json;
}

Json::Value dump_ml_params(HestonParamsML params) {
	auto json = dump_heston_params(params);
	// simulation params
	Json::Value simulation;
	simulation["ml_start_level"] = params.ml_start_level;
	simulation["ml_constant"] = params.ml_constant;
	simulation["ml_path_cnt_start"] = params.ml_path_cnt_start;
	simulation["ml_epsilon"] = params.ml_epsilon;
	json["simulation_ml"] = simulation;
	return json;
}

unsigned asHex(Json::Value val) {
	return std::stoul(val.asString(), 0, 0);
}


