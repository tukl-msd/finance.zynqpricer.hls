//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#ifndef __HESTON_ML_ACC_HPP
#define __HESTON_ML_ACC_HPP

#include "json/json-forwards.h"

#include "heston_types.hpp"

float heston_ml_hw(Json::Value bitstream, HestonParamsML ml_params);

#endif

