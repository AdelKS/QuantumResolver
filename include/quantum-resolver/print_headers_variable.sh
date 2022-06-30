#!/bin/bash
echo "quantum_headers = files("
for f in $(find * -type f -name \*.h)
do
echo "    '$f',"
done
echo ")"
