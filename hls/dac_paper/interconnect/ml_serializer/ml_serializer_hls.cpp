//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 24. November 2013
//
// Using: Xilinx Vivado HLS 2013.2
//

#include <hls_stream.h>
#include <hls_math.h>
#include <ap_axi_sdata.h>

template<int D, int TI>
struct axis{
	ap_int<D>    data;
	ap_uint<1>   last;
	ap_uint<TI>  id;
};

#define MAX_COARSE_STEP_CNT 16384 // smaller than 2**16 = 65536, due to buffer_index size

void ml_serializer(hls::stream<axis<32,1> > &s_fine,
			hls::stream<axis<32,1> > &s_coarse,
			hls::stream<axis<32,1> > &s_out) {
	#pragma HLS interface ap_fifo port=s_fine
	#pragma HLS RESOURCE variable=s_fine core=AXIS \
			metadata="-bus_bundle S_FINE"
	#pragma HLS interface ap_fifo port=s_coarse
	#pragma HLS RESOURCE variable=s_coarse core=AXIS \
			metadata="-bus_bundle S_COARSE"
	#pragma HLS interface ap_fifo port=s_out
	#pragma HLS RESOURCE variable=s_out core=AXIS \
			metadata="-bus_bundle S_OUT"
	#pragma HLS interface ap_ctrl_none port=return

	#pragma HLS PIPELINE II=1

	static bool output_fine = true;
	static ap_uint<16> buffer_write_index = 0;
	static ap_uint<16> buffer_read_index = 0;
	static axis<32,1> buffer[MAX_COARSE_STEP_CNT];

	axis<32,1> res;
	bool write_res = false;

	if (output_fine) {
		axis<32,1> fine;
		if (s_fine.read_nb(fine)) {
			res = fine;
			res.id = 0;
			write_res = true;
			if (res.last)
				output_fine = false;
		}
	} else {
		if (buffer_write_index != buffer_read_index) {
			res = buffer[buffer_read_index];
			res.id = 1;
			write_res = true;
			++buffer_read_index;
			if (res.last)
				output_fine = true;
		}
	}

	axis<32,1> coarse;
	if (s_coarse.read_nb(coarse)) {
		if (buffer_write_index != buffer_read_index) {
			buffer[buffer_write_index] = coarse;
			++buffer_write_index;
		}
	}

	if (write_res) {
		s_out.write(res);
	}
}
