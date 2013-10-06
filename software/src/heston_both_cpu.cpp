//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#include "heston_both_cpu.hpp"

#include <random>
#include <thread>
#include <map>
#include <mutex>

std::mutex m_get_thread_rng;
// get separate rng for each thread
std::mt19937 *get_thread_rng() {
	std::lock_guard<std::mutex> lock(m_get_thread_rng);
	{
		static uint32_t rng_cnt = 0;
		static uint32_t seed0 = std::random_device()();
		static std::map<std::thread::id, std::unique_ptr<std::mt19937>> rng_map;

		auto it = rng_map.find(std::this_thread::get_id());
		if (it == rng_map.end()) {
			rng_map[std::this_thread::get_id()] = 
					std::unique_ptr<std::mt19937>(
					new std::mt19937(seed0 + rng_cnt));
			++rng_cnt;
		}
		return &(*rng_map[std::this_thread::get_id()]);
	}
}

#ifdef WITH_MPI
std::mt19937 *get_mpi_rng() {
	static std::mt19937 *local_rng = nullptr;
	if (local_rng == nullptr) {
		int rank, size;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		uint32_t seed0 = 0;
		if (rank == 0) 
			seed0 = std::random_device()();
		MPI_Bcast(&seed0, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		local_rng = new std::mt19937(seed0 + rank);
	}
	return local_rng;
}
#endif

/** return local random number generator
 * 
 * The local rng is seeded only once. When using threads
 * for each thread an independent rng is returned, when using
 * MPI for each process.
 *
 * The independent rngs are seeded consecutively with seed0 + n, 
 * while n goes from 0 to thread or process count - 1. seed0 is a
 * true random number.
 */
std::mt19937 *get_rng() {
#ifdef WITH_MPI
	return get_mpi_rng();
#else
	return get_thread_rng();
#endif
}
