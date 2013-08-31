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


Json::Value read_params(char *filename);

unsigned asHex(Json::Value val);


#endif
