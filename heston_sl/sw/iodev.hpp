//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#ifndef __IODEV_HPP__
#define __IODEV_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

class IODev {
public:
	IODev(unsigned base_addr, unsigned size) {
		unsigned page_size = sysconf(_SC_PAGESIZE);
		/* Open /dev/mem file */
		fd = open("/dev/mem", O_RDWR);
		if (fd < 1) {
			perror("Could not open /dev/mem");
			exit(-1);
		}
		/* mmap the device into memory */
		unsigned page_addr = (base_addr & (~(page_size-1)));
		page_offset = base_addr - page_addr;
		unsigned mem_size = base_addr + size - page_addr;
		/* round mem_size up to next page_size */
		mem_size = ((mem_size - 1) / page_size + 1) * page_size;
		ptr = mmap(NULL, mem_size, PROT_READ|PROT_WRITE, MAP_SHARED, 
			fd, page_addr);
		if (ptr == MAP_FAILED) {
			perror("Could not map memory");
			exit(-1);
		}
	}

	~IODev() {
		unsigned page_size = sysconf(_SC_PAGESIZE);
		munmap(ptr, page_size);
	}

	void *get_dev_ptr(int addr_offset) {
		return (void*)((char*)ptr + page_offset + addr_offset);
	}
private:
	int fd;
	int page_offset;
	void *ptr;
};

#endif
