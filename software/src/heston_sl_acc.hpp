//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#ifndef __HESTON_SL_ACC_HPP
#define __HESTON_SL_ACC_HPP

#include "json/json-forwards.h"

#include <stdint.h>

float heston_sl_hw(Json::Value bitstream, HestonParamsSL sl_params);

#endif

