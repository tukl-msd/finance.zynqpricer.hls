//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 30. September 2013
//

#ifndef __HESTON_BOTH_CPU_HPP__
#define __HESTON_BOTH_CPU_HPP__

#ifdef WITH_MPI
	#include "mpi.h"
#endif

#include "ziggurat/gausszig_GSL.hpp"

#include <stdint.h>

#include <random>
#include <thread>
#include <future>

#include "heston_common.hpp"


/**
 * Accumulates all log prices from all streams and calculates
 * all the multi-level metrics on the fly
 */
template<typename calc_t>
class Pricer {
public:
	Pricer(const bool do_multilevel, const HestonParams params)
		: do_multilevel(do_multilevel), params(params),
			price_mean(0), price_variance(0), price_cnt(0) {
	}

	void handle_path(calc_t fine_path, calc_t coarse_path=0) {
		calc_t val = get_payoff(fine_path) - 
				(do_multilevel ? get_payoff(coarse_path) : (calc_t) 0);
		update_online_statistics(val);
	}

	Statistics get_statistics() {
		Statistics stats;
		stats.mean = price_mean;
		stats.variance =price_variance / (price_cnt - 1);
		stats.cnt = price_cnt;
		return stats;
	}
private:
	calc_t get_payoff(calc_t path) {
		return std::max((calc_t) 0, std::exp(path) - 
				(calc_t) params.strike_price);
	}

	void update_online_statistics(double val) {
		// See Knuth TAOCP vol 2, 3rd edition, page 232
		++price_cnt;
		double delta = val - price_mean;
		price_mean += delta / price_cnt;
		price_variance += delta * (val - price_mean);
	}

	const bool do_multilevel;
	const HestonParams params;

	double price_mean;
	double price_variance;
	uint32_t price_cnt;
};


std::mt19937 *get_rng();


template<typename calc_t>
struct HestonParamsPrecalc {
	calc_t barrier_hit_correction;
	calc_t step_size;
	calc_t sqrt_step_size_fine;
	calc_t sqrt_step_size;
	calc_t log_spot_price;
	calc_t reversion_rate_TIMES_step_size;
	calc_t vol_of_vol_TIMES_sqrt_step_size_fine;
	calc_t double_riskless_rate;
	calc_t log_lower_barrier_value;
	calc_t log_upper_barrier_value;
	calc_t half_step_size;
	calc_t barrier_correction_factor;
	calc_t long_term_avg_vola;

	HestonParamsPrecalc(HestonParams p, const uint32_t step_cnt, 
			uint32_t ml_constant)
		// continuity correction, see Broadie, Glasserman, Kou (1997)
		: barrier_hit_correction(0.5826), 
		  step_size(p.time_to_maturity / step_cnt * ml_constant),
		  sqrt_step_size_fine(std::sqrt(step_size / ml_constant)),
		  sqrt_step_size(std::sqrt(step_size)),
		  log_spot_price(std::log(p.spot_price)),
		  reversion_rate_TIMES_step_size(p.reversion_rate * step_size),
		  vol_of_vol_TIMES_sqrt_step_size_fine(p.vol_of_vol * sqrt_step_size_fine),
		  double_riskless_rate(2 * p.riskless_rate),
		  log_lower_barrier_value(std::log(p.lower_barrier_value)),
		  log_upper_barrier_value(std::log(p.upper_barrier_value)),
		  half_step_size(step_size / 2),
		  barrier_correction_factor(barrier_hit_correction * sqrt_step_size),
		  long_term_avg_vola(p.long_term_avg_vola) {
	};
};


template<typename calc_t, int BLOCK_SIZE>
void get_atithetic_rn(calc_t (&z_stock)[BLOCK_SIZE], 
		calc_t (&z_vola)[BLOCK_SIZE], const unsigned upper_i,
		std::mt19937 *rng) {
	for (unsigned i = 0; i < upper_i; i += 2) {
		calc_t z1 = (calc_t) Ziggurat<double>::
				gsl_ran_gaussian_ziggurat(*rng);
		calc_t z2 = (calc_t) Ziggurat<double>::
				gsl_ran_gaussian_ziggurat(*rng);
		z_stock[i] = z1;
		z_stock[i + 1] = -z1;
		z_vola[i] = z2;
		z_vola[i + 1] = -z2;
	}
};


#if defined(_MSC_VER) or defined(__INTEL_COMPILER)
	#define INLINE __forceinline
#elif defined(__GNUC__)
	#define INLINE inline __attribute__((always_inline))
#else
	#define INLINE inline
#endif

template<typename calc_t, int BLOCK_SIZE>
INLINE void calculate_next_step(calc_t (&stock)[BLOCK_SIZE], 
		calc_t (&vola)[BLOCK_SIZE], bool (&barrier_hit)[BLOCK_SIZE],
		const calc_t (&z_stock)[BLOCK_SIZE], const calc_t (&z_vola)[BLOCK_SIZE], 
		const unsigned upper_i, const HestonParamsPrecalc<calc_t> &p) {
	// Having any struct access in our inner most loop prevents
	// msvc to vectorize our code: 6.15s instead of 5.26s (vectorized).
	// Make sure this fucntion is inlined, otherwise this is a huge overhead.
	calc_t sqrt_step_size_fine = p.sqrt_step_size_fine;
	calc_t reversion_rate_TIMES_step_size = p.reversion_rate_TIMES_step_size;
	calc_t vol_of_vol_TIMES_sqrt_step_size_fine = 
			p.vol_of_vol_TIMES_sqrt_step_size_fine;
	calc_t double_riskless_rate = p.double_riskless_rate;
	calc_t log_lower_barrier_value = p.log_lower_barrier_value;
	calc_t log_upper_barrier_value = p.log_upper_barrier_value;
	calc_t half_step_size = p.half_step_size;
	calc_t barrier_correction_factor = p.barrier_correction_factor;
	calc_t long_term_avg_vola = p.long_term_avg_vola;


	calc_t max_vola[BLOCK_SIZE];
	for (unsigned i = 0; i < upper_i; ++i) 
			max_vola[i] = std::max((calc_t) 0, vola[i]);
	calc_t sqrt_vola[BLOCK_SIZE];
	calc_t barrier_correction[BLOCK_SIZE];
	calc_t upper[BLOCK_SIZE];
	calc_t lower[BLOCK_SIZE];
	// separate vectoriable constructs
	for (unsigned i = 0; i < upper_i; ++i) {
		sqrt_vola[i] = std::sqrt(max_vola[i]);
	}
	for (unsigned i = 0; i < upper_i; ++i) {
		stock[i] += (double_riskless_rate - max_vola[i]) *
				half_step_size + sqrt_step_size_fine * sqrt_vola[i] *
				z_stock[i];
		vola[i] += reversion_rate_TIMES_step_size *
				(long_term_avg_vola - max_vola[i]) +
				vol_of_vol_TIMES_sqrt_step_size_fine * sqrt_vola[i] *
				z_vola[i];
		barrier_correction[i] = barrier_correction_factor *
				sqrt_vola[i];
		lower[i] = log_lower_barrier_value + barrier_correction[i];
		upper[i] = log_upper_barrier_value - barrier_correction[i];
	}
	for (unsigned i = 0; i < upper_i; ++i)
		barrier_hit[i] |= (stock[i] < lower[i]) | (stock[i] > upper[i]);
}


// single threaded version
template<typename calc_t, int BLOCK_SIZE>
Statistics heston_cpu_kernel_serial(const HestonParams &p, 
		const uint32_t step_cnt, const uint64_t path_cnt,
		const bool do_multilevel, const uint32_t ml_constant) {
	if (path_cnt == 0){
		Statistics zero;
		return zero;
	}
	HestonParamsPrecalc<calc_t> p_precalc(p, step_cnt, 1);
	HestonParamsPrecalc<calc_t> p_precalc_coarse(p, step_cnt, ml_constant);
	// evaluate paths
	if (BLOCK_SIZE % 2 != 0) {
		std::cout << "ERROR: block size has to be even" << std::endl;
		exit(-1);
	}
	if (do_multilevel && (step_cnt % ml_constant != 0)) {
		std::cout << "ERROR: step_cnt % ml_constant != 0" << std::endl;
		exit(-1);
	}
	std::mt19937 *rng = get_rng();
	calc_t z_stock_coarse[BLOCK_SIZE], z_vola_coarse[BLOCK_SIZE];
	Pricer<calc_t> pricer(do_multilevel, p);
	for (unsigned i = 0; i < BLOCK_SIZE; ++i)
		z_stock_coarse[i] = z_vola_coarse[i] = 0;
	for (uint64_t path = 0; path < path_cnt; path += BLOCK_SIZE) {
		// initialize
		calc_t stock[BLOCK_SIZE], stock_coarse[BLOCK_SIZE];
		calc_t vola[BLOCK_SIZE], vola_coarse[BLOCK_SIZE];
		bool barrier_hit[BLOCK_SIZE], barrier_hit_coarse[BLOCK_SIZE];
		unsigned upper_i = std::min(path_cnt - path, (uint64_t) BLOCK_SIZE);
		//TODO(brugger): benchmark only initialize fine path for ml level 0
		for (unsigned i = 0; i < upper_i; ++i) {
			stock[i] = stock_coarse[i] = p_precalc.log_spot_price;
			vola[i] = vola_coarse[i] = p.vola_0;
			barrier_hit[i] = barrier_hit_coarse[i] = false;
		}
		// calc all steps (takes 99 % of time)
		for (unsigned step = 1; step <= step_cnt; ++step) {
			calc_t z_stock[BLOCK_SIZE];
			calc_t z_vola[BLOCK_SIZE];
			// calc random numbers ( takes 70 % of time)
			get_atithetic_rn(z_stock, z_vola, upper_i, rng);
			// calculate next step (takes 30 % of time)
			calculate_next_step(stock, vola, barrier_hit, z_stock, z_vola, 
					upper_i, p_precalc);
			
			// calc coarse
			if (do_multilevel) {
				if (step % ml_constant == 0) {
					calculate_next_step(stock_coarse, vola_coarse, 
							barrier_hit_coarse, z_stock_coarse, z_vola_coarse, 
							upper_i, p_precalc_coarse);
					for (unsigned i = 0; i < upper_i; ++i)
						z_stock_coarse[i] = z_vola_coarse[i] = 0;
				} else {
					for (unsigned i = 0; i < upper_i; ++i) {
						z_stock_coarse[i] += z_stock[i];
						z_vola_coarse[i] += z_vola[i];
					}
				}
			}
		}
		// get price of option
		for (unsigned i = 0; i < upper_i; ++i) {
			calc_t log_spot = barrier_hit[i] ? 
					-std::numeric_limits<double>::infinity() : stock[i];
			calc_t log_spot_coarse = 0;
			if (do_multilevel)
				log_spot_coarse = barrier_hit_coarse[i] ? 
						-std::numeric_limits<double>::infinity() : 
						stock_coarse[i];
			pricer.handle_path(log_spot, log_spot_coarse);
		}
	}
	// payoff price and statistics
	double discount_factor = std::exp(-p.riskless_rate * p.time_to_maturity);
	return pricer.get_statistics() * discount_factor;
}


template<typename calc_t>
Statistics heston_cpu_kernel(const HestonParams &p, 
		const uint32_t step_cnt, const uint64_t path_cnt,
		const bool do_multilevel, const uint32_t ml_constant) {
#ifdef WITH_MPI
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	uint64_t local_path_cnt = path_cnt / size + 
			(path_cnt % size > rank ? 1 : 0);
	Statistics local_stats = heston_cpu_kernel_serial<calc_t, 64>(p, step_cnt, 
			local_path_cnt, do_multilevel, ml_constant);

	// combine statistics
	Statistics stats;
	Statistics stats_vec[size];
	MPI_Gather(&local_stats, sizeof(local_stats), MPI_BYTE, 
			&stats_vec, sizeof(local_stats), MPI_BYTE, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		for (int i = 0; i < size; ++i) {
			stats += stats_vec[i];
		}
	}
	return stats;
#else
	int nt = std::thread::hardware_concurrency();
	std::vector<std::future<Statistics> > f;
	for (int i = 0; i < nt; ++i) {
		uint64_t local_path_cnt = path_cnt / nt + (path_cnt % nt > i ? 1 : 0);
		f.push_back(std::async(std::launch::async, 
				heston_cpu_kernel_serial<calc_t, 64>, p, step_cnt, 
				local_path_cnt, do_multilevel, ml_constant));
	}
	Statistics stats;
	for (int i = 0; i < nt; ++i) {
		stats += f[i].get();
	}
	return stats;
#endif
}


template<typename calc_t>
Statistics heston_cpu_kernel_sl(const HestonParams &p, 
		const uint32_t step_cnt, const uint64_t path_cnt) {
	return heston_cpu_kernel<calc_t>(p, step_cnt, path_cnt, false, 1);
}


#endif
