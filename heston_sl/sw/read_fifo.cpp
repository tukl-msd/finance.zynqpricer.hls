//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//

#include "iodev.hpp"

#include <iostream>

// address of AXI Stream FIFO
const int AXI_FIFO_BASE_ADDR   = 0x4AA00000;

int main(int argc, char *argv[])
{
	IODev axi_ctrl(AXI_FIFO_BASE_ADDR);
	// Receive Data FIFO Occupancy (RDFO)
	volatile unsigned &rdfo = *((unsigned*)axi_ctrl.get_dev_ptr(0x1C));
	// Receive Data FIFO Data (RDFD)
	volatile unsigned &rdfd = *((unsigned*)axi_ctrl.get_dev_ptr(0x20));

	unsigned avail = rdfo & 0x7fffffff;
	for (unsigned i = 0; i < avail; ++i) {
		unsigned data = rdfd;
		std::cout << i << ": \t" << data << " - " << (float)data << 
				std::endl;
	}
	std::cout << "Read " << avail << " words." << std::endl;
	avail = rdfo & 0x7fffffff;
	std::cout << "Data available: " << avail << std::endl;
	return 0;
}


