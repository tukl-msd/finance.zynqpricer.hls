software
========

Linux drivers for the accelerators and optimized CPU implementations for Heston Monte Carlo simulations.

Also contains Zynq demo.

How to run the Zynq demo:
-------------------------

First build all necssary files for the demo with:
```
./clean-zynq-build.sh
```

Then run the demo with:
```
./run-zynq-demo.sh
```

How to build:
-------------

### Unix ###

Just run
```
./new-unix-build.sh
```

This will build and install all the binaries to the directory: ```./bin/```

### Windows ###

Open a console in this folder and run:
```
create-msvc11-prj.bat
```

This will create a new Microsoft Visual Studio 2012 project in the folder ```build-msvc``` To create a second project you can specify a folder suffix as a parameter. Then open the solution file.

In the project switch from Debug to Release and set ```run_cpu``` as the startup project. Adapt the run parameters and specify a parameter file, e.g. ```params_msvc.json```, as first argument. Then you can build and run the CPU benchmark.

Make sure your Machine supports AVX instructions, otherwise the executable will crash. In that case you have to disable this specific compiler flag.


Dependencies Zynq and Unix:
---------------------------

All dependencies can be installed via the package manager. For Linaro Ubuntu run:
```
sudo apt-get install g++ make cmake
```

### G++ 4.7 ###

The main compiler. Has to support most of the new C++11 features. The command ```g++``` has to be in ```$PATH```.

*tested with version 4.7.3*

### Make ###

Backend build system. The command ```make``` has to be in ```$PATH```.

*tested with version 3.81*

### CMake 2.8.0 ###
  
The main build system. The command ```cmake``` has to be in ```$PATH```.

*tested with version 2.8.10.1*


Dependencies Windows:
---------------------

### Microsoft Visual Studio 2012 ###

The main compiler on Windows. It supports most of the new C++11 features.

*tested with Update 3*
