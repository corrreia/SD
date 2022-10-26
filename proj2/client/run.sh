#!/bin/bash

echo "----Running proj2"
echo "--MAKE CLEAN--"
make clean
echo "--MAKE--"
make
echo "--RUNNING--"
./binary/tree_client 127.0.0.1:1234