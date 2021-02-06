#!/bin/sh
#合并静态库
echo combine lib
mkdir -p tmp
cd tmp
ar x ../lib/liblibevent_cpp.a
ar x ../External/libevent-2.1.8-stable/out/lib/libevent.a
ar cru libcevent_cpp.a *.o
ranlib libcevent_cpp.a
cp libcevent_cpp.a ../lib/
rm ../lib/liblibevent_cpp.a



