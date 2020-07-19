#!/bin/bash

p=0

echo "Start creating a room"
cd build

../create/create "$@" | (
    read line1
    read line2

    echo "KEYs" $line1 $line2

    trap 'kill $(jobs -p)' EXIT

    echo "start client 0 $line"
    ./main "$@" $line1 |& tee ../0.log &

    echo "start client 1 $line"
    ./main "$@" $line2 >& ../1.log
    # gdb --batch -x ../gdbcmds --args ./main "$@" $line2

    wait
)
