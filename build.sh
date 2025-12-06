#!/bin/env bash

set -xe
CFLAGS=" -Wall -Wextra -ggdb  -Wno-missing-braces"

cc $CFLAGS -Iinclude -o main src/main.c src/game.c -lX11 -lXrandr -lm -lpulse -lpulse-simple