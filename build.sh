#!/bin/sh

cd app
mkdir ../build
g++ -g -I. -std=c++17 -o ../build/main *.cc **/*.cc -lcrypto -lssl
