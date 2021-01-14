#!/bin/bash
#
# This is the utility to install muparser for the PACS examples
#
# create the build directory
mkdir -p build
#
# go in the build directory (remove all old files in that dir if needed)
cd build/
#
# run cmake
#
# this creates the dynamic libraries and optimised code
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=../../../Examples/ ../

# uncomment this line (and comment the previous one)
#if you want to debug, this also creates the static libraries

#cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=../../../Examples/ ../

#compile
make -j 2
# Remove old versions of the library (with -f rm does not complain if the file does not exist)
rm -f ../../../Examples/lib/libmuparser.a
rm -f ../../../Examples/lib/libmuparser.so

#install
make install
# remove build directory
cd ..
rm -r -f build/
