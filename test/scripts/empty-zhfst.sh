#!/bin/bash

if test -x ./hfst-ospell ; then
    touch empty.zhfst
    if ! cat $srcdir/test.strings | ./hfst-ospell -v empty.zhfst ; then
        exit 1
    fi
else
    echo ./hfst-ospell not built
    exit 77
fi

