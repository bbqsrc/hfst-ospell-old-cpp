#!/bin/bash

if test -x ./hfst-ospell ; then
    if ! cat test.strings | ./hfst-ospell errmodel.default.hfst acceptor.default.hfst ; then
        exit 1
    fi
else
    exit 73
fi

