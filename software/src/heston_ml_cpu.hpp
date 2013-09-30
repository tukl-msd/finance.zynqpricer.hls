//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#ifndef __HESTON_ML_CPU_HPP__
#define __HESTON_ML_CPU_HPP__

#ifdef WITH_MPI
	#include "mpi.h"
#endif

#include "heston_common.hpp"

template<typename calc_t>
calc_t heston_ml_cpu(const HestonParamsML p) {
	return 0;
}

#endif
