#!/bin/env bash

set -xe
CFLAGS=" -Wall -Wextra -ggdb  -Wno-missing-braces"

cc $CFLAGS -o main main.c game.c -lX11 -lXrandr -lm -lpulse -lpulse-simple