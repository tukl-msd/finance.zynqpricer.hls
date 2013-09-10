//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 10. September 2013
//

#ifndef __HESTON_BOTH_ACC_HPP__
#define __HESTON_BOTH_ACC_HPP__

#include "json/json-forwards.h"

/** 
 * start heston accelerator
 *
 * write 32 bit alligned params with size params_size 64 bit alinged
 * to accelerator
 */
void start_heston_accelerator(const Json::Value heston, void *params, 
		unsigned params_size);

#endif

