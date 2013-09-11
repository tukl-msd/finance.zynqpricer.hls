//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#ifndef __IODEV_HPP__
#define __IODEV_HPP__

#include "json_helper.hpp"

#include "json/json.h"

#include <unistd.h>

#include <iostream>
#include <vector>


class IODev {
public:

	IODev(unsigned base_addr, unsigned size);

	~IODev();

	inline volatile void *get_dev_ptr(int addr_offset) {
		return (volatile void*)((volatile char*)ptr + 
				page_offset + addr_offset);
	}

	// define move semantic
	IODev(IODev &&);

private:
	int fd;
	int page_offset;
	unsigned mem_size;
	volatile void *ptr;

	// prevent copying
	IODev(const IODev&) = delete;
	IODev &operator=(const IODev&) = delete;
};




template <typename T>
class read_fifos_iterator {
public:
	read_fifos_iterator(const std::vector<Json::Value> fifos, 
			const unsigned cnt, const useconds_t sleep_usec=50) 
		: total_read_cnt(cnt), 
		  sleep_usec(sleep_usec),
		  device_cnt(fifos.size())
	{
		if (sizeof(T) != 4) {
			std::cerr << "ERROR: only sizeof(T) = 4 supported" 
					<< std::endl;
			exit(-1);
		}
		int i = 0;
		for (auto fifo: fifos) {
			devices.push_back(IODev(asHex(fifo["Offset Address"]), 
						asHex(fifo["Range"])));
		}
	}

	void set_new_read_cnt(const unsigned new_cnt) {
		total_read_cnt = new_cnt;
	}


	bool next(T &out) {
		unsigned fifo_index;
		return next(out, fifo_index);
	}


	bool next(T &out, unsigned &fifo_index) {
		if (words_read < total_read_cnt) {
			while (avail == 0) {
				// go to next device
				current_device = (current_device + 1) % device_cnt;
				avail = get_occupancy(devices[current_device]);
				if (avail == 0) {
					usleep(sleep_usec);
				}
			}
			--avail;
			++words_read;
			out = read_data(devices[current_device]);
			fifo_index = current_device;
			return true;
		} else {
			return false;
		}
	}

private:
	// get number of words available to read for fifo device
	unsigned get_occupancy(IODev &dev) {
		// Receive Data FIFO Occupancy (RDFO)
		unsigned rdfo = *((volatile unsigned*)dev.get_dev_ptr(0x1C));
		return rdfo & 0x7fffffff;
	}

	// read next word from fifo device
	// Warning: reading from a device when no data is available 
	//          can lead to data corruption or even deadlock
	T read_data(IODev &dev) {
		// Receive Data FIFO Data (RDFD)
		return *((volatile T*)dev.get_dev_ptr(0x20));
	}

private:
	std::vector<IODev> devices;
	unsigned total_read_cnt;
	unsigned device_cnt;
	useconds_t sleep_usec;

	// current device
	unsigned current_device = 0;
	// how many words are available for current device
	unsigned avail = 0;
	// number of words read
	unsigned words_read = 0;
};

#endif

