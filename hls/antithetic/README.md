Antithetic Path Generation
==========================

Generates variance reducing antithetic path for normal distributed random numbers.
For each Monte carlo sample path given by the random numbers 
(w_stock_1 .. w_stock_N, w_vola_1 .. w_vola_N) it also generates the 
the sample path (-w_stock_1 .. -w_stock_N, -w_vola_1 .. -w_vola_N). 
This reduces the number of normal random numbers necessary for the simulation
and additionally improves the accuracy. The technique is further described on 
[wikipedia](http://en.wikipedia.org/wiki/Antithetic_variates).

```
    +-------------------+    
    |     Antithetic    |    
    |-------------------|  AXI Stream      
    | Generate variance +===>
===>|   reducing anti-  | 
    |    thetic path    +===>
    +-------------------+    
```
