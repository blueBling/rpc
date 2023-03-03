#!/bin/bash
SRC_DIR=./
DST_DIR=../protocol

#C++
rm -rf DST_DIR/*
protoc -I=$SRC_DIR --cpp_out=$DST_DIR/ $SRC_DIR/*.proto
