#!/bin/bash

make ispalindrom
printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

./ispalindrom -s -i -outfile ./output.txt ./input1.txt ./input2.txt ./input3.txt

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"
make clean