High Level Synthesis Cores
==========================

Accelerators written with Vivado HLS for the Heston pipeline.
Synthesized Cores can be found in the [ip folder](../ip).

Single-Level Heston Pipeline
----------------------------

```
         ^ AXI Slave                                                                ^ AXI Slave
         |                                                                          |
+--------+---------+    +-------------------+    +-------------------+    +--------+----------+
| Mersenne Twister |    |       ICDF        |    |     Antithetic    |    | Heston Kernel SL  |
|------------------|    |-------------------|    |-------------------|    |-------------------|
|  Uniform random  |    | Uniform to normal |    | Generate variance +===>| Calculate single- |  AXI Stream
| number generator +===>|    distributed    +===>|   reducing anti-  |    |   level Heston    +===>
|  (array version) |    |   random nnmbers  |    |    thetic paths   +===>| Monte Carlo paths |  
+------------------+    +-------------------+    +-------------------+    +-------------------+
```

See the subfolders for more details.
