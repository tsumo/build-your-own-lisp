#!/bin/bash

if [ -z "$1" ]; then
    echo "No arguments supplied"
    exit 1
fi

if [ ! -f "$1" ]; then
    echo "File not found"
    exit 2
fi

./build.sh $1 && ./bin/${1%.*}

