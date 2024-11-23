#!usr/bin/env bash
git submodule init
git submodule update --init --recursive
mkdir build
cd build
cmake -G Ninja ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .
cd ..
