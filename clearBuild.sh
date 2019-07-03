#!/bin/sh
# make clean, make操作


cd Debug
rm -rf *
cd ..
sh build.sh

cp build_test.sh ./Debug/bin/test/
cp build_test.sh ./Debug/bin/sample/