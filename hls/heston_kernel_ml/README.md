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

Return values
-------------

The data returned by the kernel is structured as following:
```
AXI Stream OUT: 32-bit float
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
The size of each of these blocks is given by the block size. When
the parameter do_multilevel is false, only fine paths are returned. The 
coarse path corresponding to the fine path can be found in consecutive
blocks.

The overall number of paths returned is rounded up to the next block
size.
