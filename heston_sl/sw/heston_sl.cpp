//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cmath>

const unsigned PACKET_SIZE = 500;
const useconds_t BUSY_WAIT_NANO_SLEEP_TIME = 0; // nano seconds
const useconds_t BUSY_WAIT_MICRO_SLEEP_TIME = 50; // micro seconds

// address of AXI Stream FIFO
const unsigned AXI_FIFO_BASE_ADDR   = 0x4AA00000;
const unsigned AXI_FIFO_SIZE        = 0x00001000;
// address of Random Number Generator
const unsigned AXI_RNG_BASE_ADDR    = 0x43C00000;
const unsigned AXI_RNG_SIZE         = 0x00002000;
// address of Heston Kernel
const unsigned AXI_HESTON_BASE_ADDR = 0x43C10000;
const unsigned AXI_HESTON_SIZE      = 0x00000080;


// sleep for nano_sec micro seconds with busy loop
void nano_sleep(long nano_sec) {
	auto start = std::chrono::steady_clock::now();
	while (true) {
		long nano = std::chrono::duration<long, std::nano>(
			std::chrono::steady_clock::now() - start).count();
		if (nano_sec <= nano)
			break;
	}
}


void call_write_thread(const unsigned cnt, const bool do_busy_wait) {
	IODev axi_ctrl(AXI_FIFO_BASE_ADDR, AXI_FIFO_SIZE);
	// Transmit Data FIFO Vacancy (TDFV)
	volatile unsigned &tdfv = *((unsigned*)axi_ctrl.get_dev_ptr(0x0C));
	// Transmit Data FIFO Data (TDFD)
	volatile unsigned &tdfd = *((unsigned*)axi_ctrl.get_dev_ptr(0x10));
	// Transmit Length FIFO (TLF)
	volatile unsigned &tlf = *((unsigned*)axi_ctrl.get_dev_ptr(0x14));

	unsigned cnt_todo = cnt;
	unsigned free = 0;
	for (unsigned i = 0; i < cnt; ++i) {
		while (free == 0) {
			free = tdfv;
			if (do_busy_wait)
				nano_sleep(BUSY_WAIT_NANO_SLEEP_TIME);
			else
				usleep(BUSY_WAIT_MICRO_SLEEP_TIME);
		}
		tdfd = i;
		--free;
		if ((i+1) % PACKET_SIZE == 0) {
			tlf = PACKET_SIZE * 4;
			cnt_todo -= PACKET_SIZE;
		}
	}
	if (cnt_todo > 0) {
		tlf = cnt_todo * 4;
	}
}


//TODO(brugger): use void* for out and cnt in bytes instead of sizeof(unsigned)
void call_read_thread(const unsigned cnt, unsigned *out, bool do_busy_wait) {
	IODev axi_ctrl(AXI_FIFO_BASE_ADDR, AXI_FIFO_SIZE);
	// Receive Data FIFO Occupancy (RDFO)
	volatile unsigned &rdfo = *((unsigned*)axi_ctrl.get_dev_ptr(0x1C));
	// Receive Data FIFO Data (RDFD)
	volatile unsigned &rdfd = *((unsigned*)axi_ctrl.get_dev_ptr(0x20));

	unsigned avail = 0;
	for (unsigned i = 0; i < cnt; ++i) {
		while (avail == 0) {
			avail = rdfo & 0x7fffffff;
			if (do_busy_wait)
				nano_sleep(BUSY_WAIT_NANO_SLEEP_TIME);
			else
				usleep(BUSY_WAIT_MICRO_SLEEP_TIME);
		}
		out[i] = rdfd;
		--avail;
	}
}

// continuity correction, see Broadie, Glasserman, Kou (1997)
#define BARRIER_HIT_CORRECTION 0.5826


struct HestonParamsHW {
	// call option
	float log_spot_price;
	float reversion_rate_TIMES_step_size;
	float long_term_avg_vola;
	float vol_of_vol_TIMES_sqrt_step_size;
	float double_riskless_rate; // = 2 * riskless_rate
	float vola_0;
    	// both knockout
	float log_lower_barrier_value;
	float log_upper_barrier_value;
	// simulation params
	uint32_t step_cnt;
	float step_size; // = time_to_maturity / step_cnt
	float half_step_size; // = step_size / 2
	float sqrt_step_size; // = sqrt(step_size)
	// = BARRIER_HIT_CORRECTION * sqrt_step_size
	float barrier_correction_factor; 
	uint32_t path_cnt;
};


float heston_sl_hw(
		// call option
		float spot_price,
		float reversion_rate,
		float long_term_avg_vola,
		float vol_of_vol,
		float riskless_rate,
		float vola_0,
		float correlation,
		float time_to_maturity,
		float strike_price,
		// both knowckout
		float lower_barrier_value,
		float upper_barrier_value,
		// simulation params
		uint32_t step_cnt,
		uint32_t path_cnt)
{
	// get device pointers
	IODev axi_heston(AXI_HESTON_BASE_ADDR, AXI_HESTON_SIZE);
	volatile unsigned &acc_ctrl = *((unsigned*)axi_heston.get_dev_ptr(0x00));

	// make sure heston accelerator is not running
	bool acc_idle = acc_ctrl & 0x4;
	if (!acc_idle) {
		std::cerr << "Heston accelerator is already running." << std::endl;
		exit(EXIT_FAILURE);
	}

	// make sure random number generator is running
	IODev axi_rng(AXI_RNG_BASE_ADDR, AXI_RNG_SIZE);
	bool rng_idle = *(unsigned*)(axi_rng.get_dev_ptr(0x00)) & 0x4;
	if (rng_idle) {
		std::cerr << "Start the Random Number generator before running"
			<< " this application" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// setup read thread
	float prices[path_cnt];
	std::thread t_read(call_read_thread, 
		path_cnt, reinterpret_cast<unsigned*>(prices), false);

	// define heston hw parameters
	float step_size = time_to_maturity / step_cnt;
	float sqrt_step_size = std::sqrt(step_size);
	HestonParamsHW params_hw = {
		std::log(spot_price),
		reversion_rate * step_size,
		long_term_avg_vola,
		vol_of_vol * sqrt_step_size,
		2 * riskless_rate,
		vola_0,
		std::log(lower_barrier_value),
		std::log(upper_barrier_value),
		step_cnt,
		step_size,
		step_size / 2,
		sqrt_step_size,
		(float) BARRIER_HIT_CORRECTION * sqrt_step_size,
		path_cnt};

	// send parameters to accelerator, 64 bit aligned
	for (unsigned i = 0x14, j = 0; i <= 0x7c; i = i + 8, ++j) {
		*((unsigned*)axi_heston.get_dev_ptr(i)) = 
				*((unsigned*)&params_hw + j);
	}

	// start heston accelerator
	acc_ctrl = 1;
	
	/* debug output
	std::cout << acc_ctrl << std::endl;
	std::cout << acc_ctrl << std::endl;
	
	for (unsigned i = 0x14; i <= 0x7c; i = i + 8) {
		void *hw_data = axi_heston.get_dev_ptr(i);
		std::cout << "hw: 0x" << std::hex << i << ": " << std::dec;
		std::cout << *((uint32_t*)hw_data) << " - " 
				<< *((float*)hw_data) << std::endl;
	}
	*/
	
	// wait until all data is available
	t_read.join();

	// calculate result
	double result = 0;
	for (uint32_t i = 0; i < path_cnt; ++i) {
		result += std::max(0.f, std::exp(prices[i]) - strike_price);
	}
	result *= std::exp(-riskless_rate * time_to_maturity) / path_cnt;
	return result;
}



void array_stream_cpu(double a, double b, double *out, unsigned out_len) {
	for (unsigned i = 0; i < out_len; ++i) {
		out[i] = 0.2 * 2.5 + i;
	}
}


void print_duration(std::chrono::steady_clock::time_point start, 
		std::chrono::steady_clock::time_point end, unsigned cnt) {
	double duration = std::chrono::duration<double>(
		end - start).count();
	std::cout << "Calculated " << cnt << " values in " << duration
		<< " seconds (" << cnt / duration << " values / sec)"
		<< std::endl;
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <path_cnt>" << std::endl;
		return -1;
	}
	uint32_t cmd_path_cnt = atoi(argv[1]);

	// heston params
	float spot_price = 100;
	float reversion_rate = 0.5;
	float long_term_avg_vola = 0.04;
	float vol_of_vol = 1;
	float riskless_rate = 0;
	float vola_0 = 0.04;
	float correlation = 0;
	float time_to_maturity = 1;
	float strike_price = 100;
	// both knowckout
	float lower_barrier_value = 90;
	float upper_barrier_value = 110;
	// simulation params
	uint32_t step_cnt = 1000;
	uint32_t path_cnt = cmd_path_cnt; // 100;

	// benchmark
	auto start_cpu = std::chrono::steady_clock::now();
//	float result_cpu = heston_sl_cpu(a, b, out_cpu, cnt);
	auto start_acc = std::chrono::steady_clock::now();
	float result_acc = heston_sl_hw(spot_price, reversion_rate,
		long_term_avg_vola, vol_of_vol, riskless_rate, vola_0,
		correlation, time_to_maturity, strike_price, lower_barrier_value,
		upper_barrier_value, step_cnt, path_cnt);
	auto end = std::chrono::steady_clock::now();

	float ref_price = 0.74870;
	float ref_price_precision = 0.00001;

	std::cout << "REF: result = " << ref_price << std::endl;
//	std::cout << "CPU: result = " << result_cpu << std::endl;
	std::cout << "CPU: "; print_duration(start_cpu, start_acc, path_cnt);
	std::cout << "ACC: result = " << result_acc << std::endl;
	std::cout << "ACC: "; print_duration(start_acc, end, path_cnt);

	std::cout << "done" << std::endl;
	return 0;
}


