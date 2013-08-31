//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#ifndef __IODEV_HPP__
#define __IODEV_HPP__

class IODev {
public:

	IODev(unsigned base_addr, unsigned size);

	~IODev();

	inline void *get_dev_ptr(int addr_offset) {
		return (void*)((char*)ptr + page_offset + addr_offset);
	}

private:
	int fd;
	int page_offset;
	void *ptr;
};

#endif

