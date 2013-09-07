Mersenne Twister - Random number generator
==========================================

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
