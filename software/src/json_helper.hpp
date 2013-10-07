//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 29. August 2013
//

#ifndef __JSON_HELPER__
#define __JSON_HELPER__

#include "json/json-forwards.h"

#include "heston_common.hpp"

/* general method to read json data from filename */
Json::Value read_params(char *filename);

/* read json data and convert it to heston single-level params */
HestonParamsSL get_sl_params(Json::Value json);

/* read json data and convert it to heston multi-level params */
HestonParamsML get_ml_params(Json::Value json);

/* read json data and convert it to heston multi-level params */
HestonParamsEval get_eval_params(Json::Value json);

unsigned asHex(Json::Value val);

#endif
