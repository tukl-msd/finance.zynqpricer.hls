//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 20. August 2013
//
// Using: Xilinx Vivado HLS 2013.2
//


//forum: http://forums.xilinx.com/t5/High-Level-Synthesis-HLS/Vivado-HLS-AXI4-address-assignment/td-p/257474/page/2
//source: http://forums.xilinx.com/xlnx/attachments/xlnx/hls/268/1/example.cpp


/*******************************************************************************
Vendor: Xilinx
Associated Filename: example.cpp
Purpose: Example to create an AXI4 master interface using Vivado HLS
Revision History:

*******************************************************************************
Copyright 2008 - 2012 Xilinx, Inc. All rights reserved.

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law:
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising under
or in connection with these materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage (including loss of data,
profits, goodwill, or any type of loss or damage suffered as a result of any
action brought by a third party) even if such damage or loss was reasonably
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES.

*******************************************************************************/

/*
 * This file contains an example for creating an AXI4-master interface using Vivado HLS
 */

#include <stdio.h>
#include <string.h>

#define N 16

void amv(volatile int *a, unsigned int byte_rdoffset, unsigned int byte_wroffset, int valueadded){

    //
    // AXI4 Master
    //

//ap_bus is the only valid native Vivado HLS interface for memory mapped master ports
#pragma HLS INTERFACE ap_bus port=a

  //Port a is assigned to an AXI4-master interface
#pragma HLS RESOURCE core=AXI4M variable=a

    //
    // AXI4 Lite
    //
#pragma HLS RESOURCE core=AXI4LiteS variable=return metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=byte_rdoffset
#pragma HLS RESOURCE core=AXI4LiteS    variable=byte_rdoffset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=byte_wroffset
#pragma HLS RESOURCE core=AXI4LiteS    variable=byte_wroffset metadata="-bus_bundle LITE"

#pragma HLS INTERFACE ap_none register     port=valueadded
#pragma HLS RESOURCE core=AXI4LiteS    variable=valueadded metadata="-bus_bundle LITE"


  int i;
  int buff[N];

  //memcpy creates a burst access to memory
  //multiple calls of memcpy cannot be pipelined and will be scheduled sequentially
  //memcpy requires a local buffer to store the results of the memory transaction

  memcpy(buff,(int *)(a+byte_rdoffset/4),N*sizeof(int));

  for(i=0; i < N; i++) {
    buff[i] = buff[i] + valueadded;
  }

  memcpy((int *)(a+byte_wroffset/4),buff,N*sizeof(int));
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*

// forum: http://forums.xilinx.com/t5/High-Level-Synthesis-HLS/Vivado-create-a-simple-AXI4-Master-IP-core/td-p/297555
// source: http://forums.xilinx.com/xlnx/attachments/xlnx/hls/599/1/main.c


//
//Vivado HLS Source
//
#include "ap_cint.h"

#define N 1024

typedef int DT;
void amv (DT* c, volatile DT *m) {

	// Define the RTL interfaces
	#pragma HLS interface ap_ctrl_none port=return
	#pragma HLS interface ap_bus port=m
	// Define the pcore interface as an AXI4 master
	#pragma HLS resource core=AXI4M variable=m

	// Define extra AXI4Lite slave port for controlling purposes 
	#pragma HLS interface ap_hs port=c
	#pragma HLS resource core=AXI4LiteS port=c

	DT buff[N], tmp;
	int i, j;

	if(*c==1)
	{
		memcpy(buff, (void*) m, N * sizeof(DT));

		loop:for (i = 0, j = N - 1; i < j; i++, j--) {
			tmp = buff[i];
			buff[i] = buff[j];
			buff[j] = tmp;
		}

		memcpy((void*) m, buff, N * sizeof(DT));
	}
	*c = 0;
}
*/
