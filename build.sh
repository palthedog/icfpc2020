#!/bin/sh

cd app
mkdir ../build
g++ -Wall -g -I. -std=c++17 -o ../build/main *.cc **/*.cc -lcrypto -lssl
#g++ -O3 -I. -std=c++17 -o ../build/main *.cc **/*.cc -lcrypto -lssl
