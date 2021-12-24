#!/bin/sh
# Copyright (c) 2021 Olle Lögdahl
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

# Runs a test (*.t) using it's associated data (*.d) if it exists.

BFC=out/bfc
COMPILE_DIR=${mktemp}

test_single() {
    echo -ne "$1:\t"
    $BFC -o "$COMPILE_DIR/$1" $TEST 2>$COMPILE_DIR/log || {
        echo "FAILED compile of test $1:";
        cat $COMPILE_DIR/log;
        exit 1;
    }

    echo "success";

    [ -f TEST_DAT ] && {
        echo "found";
    } || {
        echo "not found";
    }
}

echo "TEST v0.0.1"

# ensure that bfc exists.
[ -f $BFC ] || { echo "out/bfc does not exist. aborting."; exit 1; }
# ensure that test directory exists.
[ -d tests/ ] || { echo "directory tests/ does not exist. aborting."; exit 1; }

[ $# -gt 0 ] && {
    for name in "$@"; do test_single "$name"; done;
} || {
    for name in "$(find tests/ -name '*.t')"; do test_single "$name"; done;
}
