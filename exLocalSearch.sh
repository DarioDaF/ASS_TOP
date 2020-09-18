#!/bin/sh

## PREPARATION ##
# make clean
make

## EXECUTE ALL ISTANCES ## 
# Solve all istances and with the specified parameters and print the results

# Version 1.1 : execute the specified algorithm to all the instances from Greedy or backtracking best routes #
./MainLocalSearch.exe SA GB
./MainLocalSearch.exe HC GB
./MainLocalSearch.exe SD GB
./MainLocalSearch.exe TS GB

# Version 1 : execute the specified algorithm to all the instances from LocalSearch's best routes algorithms #
./MainLocalSearch.exe SA LS
./MainLocalSearch.exe HC LS
./MainLocalSearch.exe SD LS
./MainLocalSearch.exe TS LS

# Version 2 : open test and execution mode on one instance#
./MainLocal.exe