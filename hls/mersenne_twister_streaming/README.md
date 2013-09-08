Mersenne Twister - Random number generator
==========================================

---

**Warning:** *this version is broken in Vivado HLS 2013.2.*
*Use the [array version](../mersenne_twister_array) instead.*

*The design uses hls::stream which are buggy, see [this post][1]*
*on the Xilinx forum for more information about this issue.*
*The issue will be fixed in the next version of Vivado HLS.*
*Since the stream based Mersenne Twister is much easier to read*
*it will remain here in the original form.*

[1]: http://forums.xilinx.com/t5/Design-Tools-Others/HLS-Compiler-Bug-Streaming-Based-Design/td-p/349833

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
