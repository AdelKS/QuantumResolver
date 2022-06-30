#!/bin/bash
echo "quantum_sources = files("
for f in $(find * -type f -name \*.cpp ! -name quantum.cpp)
do
echo "    '$f',"
done
echo ")"
