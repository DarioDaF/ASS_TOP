#!/bin/sh

## PREPARATION ##
# make clean
make

## EXECUTE ALL ISTANCES ## 
# Solve all istances and with the specified parameters and print the results

# Version 1 : execute the specified algorithm to all the instances #
./MainLocalSearch.exe SA
./MainLocalSearch.exe HC
./MainLocalSearch.exe SD
./MainLocalSearch.exe TS

# Version 2 : open test and execution mode on one instance#
./MainLocal.exe