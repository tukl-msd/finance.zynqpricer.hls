Heston Single-Level Kernel
==========================

Generates Monte Carlo paths for the [Heston Model](http://en.wikipedia.org/wiki/Heston_model).

```
             ^ AXI Slave
             |
    +--------+----------+
    | Heston Kernel SL  |
    |-------------------|
===>| Calculate single- |  AXI Stream
    |   level Heston    +===>
===>| Monte Carlo paths |  
    +-------------------+
```

