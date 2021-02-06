#!/bin/sh
# make clean, make操作


cd Debug
rm -rf *
cd ..
sh build.sh

cp restart_server.sh bin/
cp restart_client.sh bin/
cp restart_test_bus_mgr.sh bin/