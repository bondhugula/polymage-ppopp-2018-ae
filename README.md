**LICENSE**

PolyMage is available under the Apache License version 2.0. Please see 
the LICENSE file for details.

**REQUIREMENTS**

1) Python 3.4 or higher

2) Python packages numpy, pytest. These can be installed via  
(on a Fedora) $ sudo yum -y install python3-numpy python3-pytest  
(on Ubuntu) $ sudo apt-get install python3-numpy python3-pytest  

3) OpenCV 2.4.7 or higher (with QT/GTK support, video codec support for 
the video demo),  Python bindings for OpenCV. Install instructions on 
Ubuntu: https://help.ubuntu.com/community/OpenCV  If you don't have a 
GPU on your machine, be sure to call cmake with the option -D 
WITH_CUDA=OFF  On a Fedora, these can be installed with 'sudo yum -y 
install opencv python-opencv'

4) g++ (GNU C++ compiler) version 4.8 or higher or Intel C/C++ compiler 
(icpc) (recommended: icpc 15.0 or higher)

5) Python bindings for isl: islpy http://documen.tician.de/islpy/  
This can be easily installed via python3-pip  ```
$ sudo yum -y install python3-pip  
$ sudo pip3 install islpy  
```
(islpy itself requires ffi development files -- this can be installed by 
installing libffi-devel via yum/apt-get)

6) The video demo (sandbox/video_demo) has additional requirements; see 
sandbox/video_demo/README.md

7) Boost C++ Libraries version 1.3 or higher

**INSTALLATION**

```
$ git clone git@bitbucket.org:udayb/polymage.git
$ cd polymage
$ git submodule update --init
$ cd cgen
$ git am ../patches/0001-ctye-to-dtype-handle-void.patch
$ sudo python3 setup.py install
$ cd ..
# (To build the DP-based fusion module)
# (Set your Python version in 'Makefile')
$ make

Try any of the apps in the sandbox/apps/ directory.

[Optional] Required for diamond tiling with Multigrid

$ cd libpluto/

$ (follow installation instructions for pluto)

```


**BUILD**

$ cd polymage

$ make

**PROJECT STRUCTURE**

sandbox is the main directory of interest and it contains most of the 
code.  sandbox/tests is the test directory  and has a lot of sample code 
which you can take a look at.  You can run the tests by invoking the 
following command:  

$> py.test-3 test_{name}.py  

For example, the harris corner detection test can be run using the 
following command from the  sandbox/tests directory:

.../sandbox/tests$> py.test-3 test_harris.py

Note: The input language does not exactly match that in the ASPLOS 2015 
paper; however, it is very close.  

sandbox/apps/python : has some benchmarks written using a Python driver 
code.  Here we eliminated the need for a C++ driver and manage the 
pipeline input and output in Python.  

sandbox/constructs.py : PolyMage language constructs  

sandbox/pipe.py : high level flow of the optimizer  

sandbox/poly.py : for polyhedral representation of the pipelines  

sandbox/schedule.py : schedule transformation for the computations  

sandbox/codegen.py : code generation for the scheduled pipeline  

sandbox/targetc.py : c++ code generation  

sandbox/tuner.py : autotuning code  

sandbox/libpluto.py : FFI access to PLUTO

sandbox/video_demo has a demo comparing PolyMage optimized versions of 
some of the benchmarks with other reference implementations -- it can be 
run on any video file. Trying this is strongly encouraged.

The following repository contains just the base and the best PolyMage 
optimized codes (for Intel  Sandybridge) used for experiments in the 
ASPLOS 2015 paper for all of the benchmarks -- these are  sufficient if 
one is purely interested in a final performance comparison without any 
tweaking/tuning:  
https://github.com/bondhugula/polymage-benchmarks
