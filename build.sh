#!/bin/bash

if [ -z "$1" ]; then
    echo "No arguments supplied"
    exit 1
fi

if [ ! -f "$1" ]; then
    echo "File not found"
    exit 2
fi

mkdir -p bin

FILES=(
    mpc.c
    utils.c
    lval.c
    eval.c
)

cc -std=c99 -Wall "$1" ${FILES[@]} -ledit -lm -o "./bin/${1%.*}"

