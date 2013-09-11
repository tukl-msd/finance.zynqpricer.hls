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

The data returned by the kernel is structured as following:
```
32-bit float stream
+--------+------------+--------------+-------+--------+-----------+
| 1 word |  BS words  |  BS words    |  ...  |  ...   | ...       |
|--------|------------|--------------|-------|--------|-----------|
|  BS    | log price  |  log price   |  ...  |  ...   | ...       | ...
|        | fine paths | coarse paths |  fine | coarse |           |
+--------+------------+--------------+-------+--------+-----------+
  BS = Block Size (e.g. 256)
```
All data is returned as 32-bit single-precision IEEE floating point. 
The first word returns the block size of the kernel. Typical values are
256 or 512. This followes interleaved blocks for fine and coarse paths.
Each block has a length of block size. 
