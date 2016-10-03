#!/bin/bash
echo "run test on git..."
rm -rf tmp
mkdir tmp
cd tmp

#create repo
#base1
mkdir base1
cd base1
git init --bare
cd ../
#base2
mkdir base2
cd base2
git init --bare
cd ../

name="test1"
if [ "$1" != "" ];then
    name=$1
fi
#
#tes1
echo "test1..."
mkdir test1
cd test1
git clone ../base1 base
cd base
cp ../../../test/$name.gitconfig .gitconfig
cp ../../../test/test.gitattributes .gitattributes
echo abc>data.dat
git add .
git commit -m "testing"
git push
git pull
cd ../../
echo 
#test2
echo "test2..."
mkdir test2
cd test2
git clone ../base2 base
cd base
cp ../../../test/$name.gitconfig .gitconfig
cp ../../../test/test.gitattributes .gitattributes
echo abc>data.dat
git add .
git commit -m "testing"
git push
git pull
cd ../../
echo 
#test3
echo "test3..."
mkdir test3
cd test3
git clone ../base1 base
cd base
cp ../../../test/$name.gitconfig .gitconfig
cp ../../../test/test.gitattributes .gitattributes
echo abcdefg>data.dat
git add .
git commit -m "testing2"
git push
git pull
cd ../../
echo 
#test4
echo "test4..."
cd test1/base
git pull
git lfs pull
cat data.dat
cd ../../
echo 
echo 
#test5
echo "test5..."
cd test2/base
git pull
git lfs pull
cat data.dat
cd ../../
echo 
echo 

#
cd ../

echo "all done..."