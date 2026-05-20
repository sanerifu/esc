#!/usr/bin/env sh

cc esc.c -o esc -fsanitize=address -g3

if [ $? -eq 0 ]; then
    ./esc test.esc
fi
