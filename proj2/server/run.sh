#!/bin/bash

echo "----Running proj2"
echo "--MAKE CLEAN--"
make clean
echo "--MAKE--"
make
echo "--RUNNING--"
./binary/tree_server 8888