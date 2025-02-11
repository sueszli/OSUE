#!/bin/bash

ARGS=""
ARGS="-i -o -s"
ARGS="-i -o"
ARGS="-i"
ARGS="-i test"
ARGS="test"

clear

echo -e "--------- COMPILING ---------"
gcc -o mygrep mygrep.c -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./mygrep $ARGS

echo -e "\n\n--------- CLEANING ---------"
	rm -rf *.o mygrep
