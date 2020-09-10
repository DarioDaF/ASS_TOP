#!/bin/sh

## PREPARATION ##
# make clean
make

## PARAMETER TESTER ##
# Solve son istance and allow the user to set the parameters
#
# Example of command line:
#   ./ *.exe [map] [paramToTest] [from] [to] [increment] [otherParam1] [otherParam2] [otherParam3] ...
#   Usage:
#       - [map] -> 0 to select all the maps
#       - if [paramToTest] is "C" , then [otherparam1] is "A", [otherParam2] is "B", [otherParam3] is "D" ... 

./MainTest.exe 0 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 0 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 0 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 0 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 1 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 1 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 1 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 1 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 2 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 2 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 2 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 2 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 3 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 3 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 3 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 3 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 4 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 4 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 4 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 4 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 5 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 5 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 5 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 5 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 6 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 6 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 6 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 6 D 0.1 2.0 0.1 1.0 1.0 1.0

./MainTest.exe 7 A 0.1 2.0 0.1 1.0 1.0 1.0 
./MainTest.exe 7 B 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 7 C 0.1 2.0 0.1 1.0 1.0 1.0
./MainTest.exe 7 D 0.1 2.0 0.1 1.0 1.0 1.0

## PLOT THE DATA ##
# plot with python script the variation of parameter calculated above

python3 ./plotter/plotParams.py 0 A
python3 ./plotter/plotParams.py 0 B
python3 ./plotter/plotParams.py 0 C
python3 ./plotter/plotParams.py 0 D

python3 ./plotter/plotParams.py 1 A
python3 ./plotter/plotParams.py 1 B
python3 ./plotter/plotParams.py 1 C
python3 ./plotter/plotParams.py 1 D

python3 ./plotter/plotParams.py 2 A
python3 ./plotter/plotParams.py 2 B
python3 ./plotter/plotParams.py 2 C
python3 ./plotter/plotParams.py 2 D

python3 ./plotter/plotParams.py 3 A
python3 ./plotter/plotParams.py 3 B
python3 ./plotter/plotParams.py 3 C
python3 ./plotter/plotParams.py 3 D

python3 ./plotter/plotParams.py 4 A
python3 ./plotter/plotParams.py 4 B
python3 ./plotter/plotParams.py 4 C
python3 ./plotter/plotParams.py 4 D

python3 ./plotter/plotParams.py 5 A
python3 ./plotter/plotParams.py 5 B
python3 ./plotter/plotParams.py 5 C
python3 ./plotter/plotParams.py 5 D

python3 ./plotter/plotParams.py 6 A
python3 ./plotter/plotParams.py 6 B
python3 ./plotter/plotParams.py 6 C
python3 ./plotter/plotParams.py 6 D

python3 ./plotter/plotParams.py 7 A
python3 ./plotter/plotParams.py 7 B
python3 ./plotter/plotParams.py 7 C
python3 ./plotter/plotParams.py 7 D

## PARAMETER MAXDEVIATION SCALER ## 

./MainMapper.exe

## EXECUTE ALL ISTANCES ## 
# Solve all istances and with the specified parameters and print the results

./MainSolver.exe