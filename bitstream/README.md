Zynq bitstreams
===============

Contains ready to load bitstreams for the Zynq (XC7Z020 CLG484 -1). 
The bitstreams work for both the Zedboard and the ZC702 development board.

To load them, boot up Linux and run for example:
```
cat heston_sl_6x.bin > /dev/xdevcfg
```
Reconfiguration takes less than 200 ms. 


Available bitstreams
--------------------

The available bitstreams are

###empty.bin###

An empty bitstream. When loaded it deletes all the configurtions from the FPGA.

###heston_sl_3x.bin###

Contains three Heston single-level pipelines based on the Box Muller transformation. 
All devices are attached to GP0 and run up to a frequency of 100 MHz.

![FPGA floorplan of heston_sl_3x.bin](https://git.rhrk.uni-kl.de/EIT-Wehn/finance.zynqpricer.hls/raw/master/bitstream/heston_sl_3x.png)

###heston_sl_6x.bin###

Contains six Heston single-level pipelines based on the ICDF transformation. 
All devices are attached to GP0 and run up to a frequency of 100 MHz.

![FPGA floorplan of heston_sl_3x.bin](https://git.rhrk.uni-kl.de/EIT-Wehn/finance.zynqpricer.hls/raw/master/bitstream/heston_sl_6x.png)


