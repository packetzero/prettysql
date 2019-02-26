#!/bin/bash
TOP=`dirname $0`
mkdir -p ${TOP}/build
cd ${TOP}/build

export MAKE_TESTS=1
export GTEST_DIR=/Users/alexmalone/googletest
cmake -G Xcode -DCMAKE_CXX_FLAGS=-DPRETTYSQL_JSON=1 ..

xcodebuild -configuration Release
