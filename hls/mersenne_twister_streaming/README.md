Mersenne Twister - Random number generator
==========================================

---
**Info:** *Verified to work with Vivado HLS 2013.4. Use this version from now on.*
---

Generate uniform distributed random numbers used for Monte Carlo simulations.


```
         ^ AXI Slave
         |         
+--------+---------+
| Mersenne Twister |
|------------------|
|  Uniform random  |  AXI Stream
| number generator +===>
| (stream version) |
+------------------+
```

The seed can be configured over the memmory mapped AXI slave interface.

To generate a continous stream of random numbers it is necessary to 
to also set the auto_restart bit when starting this core.
