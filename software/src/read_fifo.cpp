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
const unsigned AXI_FIFO_BASE_ADDR   = 0x4AA00000;
const unsigned AXI_FIFO_SIZE        = 0x00001000;

int main(int argc, char *argv[])
{
	IODev axi_ctrl(AXI_FIFO_BASE_ADDR, AXI_FIFO_SIZE);
	// Receive Data FIFO Occupancy (RDFO)
	volatile unsigned &rdfo = *((volatile unsigned*)axi_ctrl.get_dev_ptr(0x1C));
	// Receive Data FIFO Data (RDFD)
	volatile unsigned &rdfd = *((volatile unsigned*)axi_ctrl.get_dev_ptr(0x20));

	unsigned avail = rdfo & 0x7fffffff;
	for (unsigned i = 0; i < avail; ++i) {
		unsigned data = rdfd;
		std::cout << i << ": \t" << data << " \t " << 
			reinterpret_cast<float&>(data) << std::endl;
	}
	std::cout << "Read " << avail << " words." << std::endl;
	avail = rdfo & 0x7fffffff;
	std::cout << "Data available: " << avail << std::endl;
	return 0;
}


