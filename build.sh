#!/bin/sh

cd app
mkdir ../build
#g++ -Wall -g -I. -std=c++17 -o ../build/main *.cc **/*.cc -lcrypto -lssl -lboost_system -lboost_thread -lpthread
g++ -O3 -I. -std=c++17 -o ../build/main *.cc **/*.cc -lcrypto -lssl -lboost_system  -lboost_thread -lpthread
