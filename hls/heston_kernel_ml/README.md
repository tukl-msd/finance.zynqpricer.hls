Heston Multi-Level Kernel
=========================

Generates Monte Carlo paths for the [Heston Model](http://en.wikipedia.org/wiki/Heston_model).

```
             ^ AXI Slave
             |
    +--------+----------+
    | Heston Kernel ML  |
    |-------------------|
===>| Calculate multi-  |  AXI Stream
    |   level Heston    +===>
===>| Monte Carlo paths |  
    +-------------------+
```

The Heston parameters can be configured over the memory mapped AXI slave interface.
