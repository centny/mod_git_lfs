#!/bin/bash
set -e
mkdir -p build/test
cp -f test/test_data.json build/test/
cd build/test
make
./test_git_lfs
cd ../../