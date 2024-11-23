#!usr/bin/env bash
mkdir build
cd build
cmake -G Ninja ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .
cd ..
