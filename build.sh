#!/bin/sh

cd app
mkdir ../build
g++ -std=c++17 -o ../build/main *.cc **/*.cc
