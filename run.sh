#!/usr/bin/env sh

cc scs.c -o scs -fsanitize=address -g3

if [ $? -eq 0 ]; then
    ./scs test.scs
fi
