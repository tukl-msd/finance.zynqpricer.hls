//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 27. August 2013
//

#include "heston_sl_cpu.hpp"

#include <random>
#include <map>
#include <mutex>

// get separate rng for each thread
std::mt19937 get_thread_rng() {
	static uint32_t rng_cnt = std::random_device()();
	static std::map<std::thread::id, std::mt19937> rng_map;
	static std::mutex m;
	{
		std::lock_guard<std::mutex> lock(m);
		auto it = rng_map.find(std::this_thread::get_id());
		if (it == rng_map.end()) {
			rng_map[std::this_thread::get_id()] = std::mt19937(rng_cnt);
			++rng_cnt;
		}
		return rng_map[std::this_thread::get_id()];
	}
}
