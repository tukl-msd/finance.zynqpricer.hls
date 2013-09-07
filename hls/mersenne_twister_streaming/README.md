Mersenne Twister - Random number generator
==========================================

**Warning:** this version is based on hls::stream which is broken in Vivado 
HLS 2013.2. Use the array version instead.

Generate uniform distributed random numbers used for Monte Carlo simulations.

```
         ^ AXI Slave
         |         
+--------+---------+
| Mersenne Twister |
|------------------|
|  Uniform random  |  AXI Stream
| number generator +===>
|  (array version) |
+------------------+
```

The seed can be configured over the memmory mapped AXI slave interface.
