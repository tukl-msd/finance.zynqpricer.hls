ICDF - Random number transformation
===================================

Transforms uniformly distributed random numbers into normal distributed
random numbers based on [inverse sampling][1] of the [normal distribtuion][2]. 
The core maps very efficiently to FPGA ressources.

[1]: http://en.wikipedia.org/wiki/Inverse_transform_sampling
[2]: http://en.wikipedia.org/wiki/Normal_distribution

```     
    +--------------------+    
    |        ICDF        |    
    |--------------------|    
    |     Uniform to     |  AXI Stream  
===>| normal distributed +===>
    |   random nnmbers   |    
    +--------------------+    
```

To generate and analyze the coefficients run:
```
python3 icdf_fit.py
```

Dependencies for icdf_fit.py:
-----------------------------

All dependencies can be installed via the package manager. For Linaro Ubuntu run:
```
sudo apt-get install python3 python3-numpy python3-scipy python3-matplotlib
```

### Python 3 ###
  
The main interpreter.

*tested with version 3.3.0*

### Numpy ###

Vector library for Python

*tested with version 1.7.1*

### SciPy ###

Python library for optimization and polynom fitting.

*tested with version 0.12.0*

#### Optional: Matplotlib ####

Plotting library for Python to plot the approxiamtion.

*tested with version 1.2.1*

