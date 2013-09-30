//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

// Optimizations:
// - pre-calculation
// - using best known gaussion transformation method: ziggurat
// - vectorizable loops (AVX)
// - exploit loop pipelining / blocking
//
// Compiler flags:
// - MSCV: /O2 /arch:AVX /fp:fast /GL
// - GCC on ARM: -O3 -march=native -ffast-math -mfpu=neon
// - GCC on Intel: -O3 -march=native -ffast-math

#ifndef __HESTON_SL_CPU_HPP__
#define __HESTON_SL_CPU_HPP__

#ifdef WITH_MPI
	#include "mpi.h"
#endif

#include "ziggurat/gausszig_GSL.hpp"

#include <stdint.h>

#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <future>

#include "heston_common.hpp"
#include "heston_both_cpu.hpp"

template<typename calc_t>
calc_t heston_sl_cpu(HestonParamsSL p, Statistics *stats_=nullptr) {
#ifdef WITH_MPI
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	uint64_t total_path_cnt = p.path_cnt;
	p.path_cnt = total_path_cnt / size + (total_path_cnt % size > rank ? 1 : 0);
	Statistics local_stats;
	double local_res = heston_cpu_kernel<calc_t, 64>(p, p.step_cnt, 
			p.path_cnt) * p.path_cnt;
	double result = 0;
	MPI_Reduce(&local_res, &result, 1, MPI_DOUBLE, MPI_SUM, 
			0, MPI_COMM_WORLD);

	// combine statistics
	if (stats != nullptr) {
		Statistics stats_vec[size];
		MPI_Gather(&local_stats, sizeof(local_stats), MPI_BYTE, 
				&stats_vec, sizeof(local_stats), MPI_BYTE, 0, MPI_COMM_WORLD);
		if (rank == 0) {
			for (int i = 0; i < size; ++i) {
				(*stats) += stats_vec[i];
			}
		}
	}

	return (calc_t)(result / total_path_cnt);
#else
	int nt = std::thread::hardware_concurrency();
	p.path_cnt /= nt;
	std::vector<std::future<Statistics> > f;
	for (int i = 0; i < nt; ++i) {
		f.push_back(std::async(std::launch::async, 
			heston_cpu_kernel_sl<calc_t, 64>, p, p.step_cnt, 
			p.path_cnt));
	}
	Statistics stats;
	for (int i = 0; i < nt; ++i) {
		stats += f[i].get();
	}
	return (calc_t)(stats.mean);
#endif
}

#endif
