#!/bin/sh

cd app
mkdir ../build
#g++ -g -I. -std=c++11 -o ../build/main *.cc **/*.cc -lcrypto -lssl
g++ -O3 -I. -std=c++11 -o ../build/main *.cc **/*.cc -lcrypto -lssl
