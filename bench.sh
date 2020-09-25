#!/bin/bash

#This bash script is used for compiling and benching the prod-cons_multithreading.c programm

#compiling
gcc timer.c myFunctions.c -pthread -o program.out -lm -O3

#benching the program
for queuesize in 1 2 4  8  10
do
  for q in 1 2 4  8  10
  do
    ./program.out "$queuesize"  "$q"
  done
done
