#!/bin/bash

p=0

cd build

../create/create "$@" | (
    read line1
    read line2

    echo "KEYs" $line1 $line2

    trap 'kill $(jobs -p)' EXIT

    ./main "$@" $line1 |& tee ../0.log &

    ./main "$@" $line2 >& ../1.log
    #gdb --batch -x ../gdbcmds --args ./main "$@" $line2

    wait
)
