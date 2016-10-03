#!/bin/bash
set -e
for((i=0;i<100000;i++));
do
    echo $i
    ./testgit.sh test2
done
