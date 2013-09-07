Box Muller Transformation
=========================

*Deprecated: Use [ICDF](../icdf) instad*

Transforms uniformly distributed random number into normal distributed ones.
More information on [wikipedia](http://en.wikipedia.org/wiki/Box_muller)

This transformation was used in the first version of the Zynq demo
because of its unbeatable brevity (10 lines of code). However the `sin`, 
`cos` and `log` operations map very bad to FPGA ressources.
