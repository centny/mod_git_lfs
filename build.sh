#!/bin/bash
set -e
find . -name \*.gcno -type f -delete
find . -name \*.gcda -type f -delete
rm -rf reports
mkdir reports
mkdir reports/html
mkdir reports/xml
mkdir reports/gcov
if [ "$1" = "" ];then
 ./autogen.sh
 export LIBCWF_CFLAGS="-fprofile-arcs -ftest-coverage"
 export LIBCWF_LDFLAGS="-fprofile-arcs -ftest-coverage"
 ./configure --prefix=`pwd`/build
fi
#
make clean
make
make install
export DYLD_LIBRARY_PATH=`pwd`/build/lib
export LD_LIBRARY_PATH=`pwd`/build/lib
cd build/bin
./testcwf
cd ../../
echo "create coverage reports"
cd src
gcovr --html --html-details -o ../reports/html/coverage.html -r `pwd` -v `pwd`
gcovr --xml -o ../reports/xml/coverage.xml -r `pwd` -v `pwd`