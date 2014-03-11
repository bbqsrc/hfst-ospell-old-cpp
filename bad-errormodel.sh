#!/bin/bash

if test -x ./hfst-ospell ; then
    if ! cat $srcdir/test.strings | ./hfst-ospell -v bad_errormodel.zhfst ; then
        exit 1
    fi
else
    echo ./hfst-ospell not built
    exit 73
fi

