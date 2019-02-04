#!/bin/bash
#
# This is the utility to install muparser for the PACS examples
#
#
# go in the build directory (remove all old files in that dir if needed)
cd build/
#
# run cmake
#
# this celates the dynamic libraries and optimised code
cmake -D CMAKE_INSTALL_PREFIX=../../../Examples/ ../

# uncomment this line (and comment the previous one)
#if you want to debug, this also creates the static libraries

#cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=../../../Examples/ ../

#compile
make
#install
make install
