#!/bin/bash

clear

make supervisor
make generator

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./generator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"
./generator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"
make clean
