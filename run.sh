#!/usr/bin/sh

cc scs.c -o scs -fsanitize=address -fsanitize=leak -g3

if [ $? -eq 0 ]; then
    ./scs test.scs
fi
