High Level Synthesis Cores
==========================

Accelerators written with Vivado HLS for the Heston pipeline.
Synthesized Cores can be found in the [ip folder](../ip)

Single-Level Heston Pipeline
----------------------------

```
          ^ AXI Slave                                                                ^ AXI Slave
          |                                                                          |
 +--------+---------+    +--------------------+    +-------------------+    +--------+----------+
 | Mersenne Twister |    |       ICDF         |    |     Antithetic    |    | Heston Kernel SL  |
 |------------------|    |--------------------|    |-------------------|    |-------------------|
 |  Uniform random  |    | Transformation to  |    | Generate variance +===>| Calculate single- |  AXI Stream
 | number generator +===>| normal distributed +===>|   reducing anti-  |    |   level Heston    +===>
 |  (array version) |    |   random nnmbers   |    |    thetic path    +===>| Monte Carlo paths |  
 +------------------+    +--------------------+    +-------------------+    +-------------------+
```
