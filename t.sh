#!/bin/bash
set -e
mkdir -p build
cd build
../configure
make
cp -f .libs/mod_git_lfs.so /etc/httpd/modules
cp -f ../10-gitlfs.conf /etc/httpd/conf.modules.d/
cp -f ../gitlfs.conf /etc/httpd/conf.d/
service httpd restart
cd ../