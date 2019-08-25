#!/bin/sh
# make clean, make操作

mkdir Debug
cd Debug
rm -rf *
cd ..
sh build.sh

cp test_build.sh ./Debug/bin/test/build.sh
cp test_build.sh ./Debug/bin/samples/build.sh