#!/bin/bash
./configure --prefix=`pwd`/Examples/
make clean
make
make install
make clean
# use this if you want shared libraries
#./configure --prefix=../../Examples/ --enable-shared
