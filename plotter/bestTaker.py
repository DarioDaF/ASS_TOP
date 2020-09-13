'''
  The function evaluate the best solution obtained from all the algorithms and take the best algorithm. Than 
  save the best path for Local Search algortim. Also Write a new ".csv" file with the best solutions found.

  Usage:
    python3 ./plotter/bestTaker.py

  Input files:
    - Sol[algorithm]#[version].csv : file ".csv" that contain the solution of one version of one algorithm.
                                     The file is located in "solutions/" directory.

  Output files:
    - bestSolutions.csv : file that contain the best solutions found, with the algorithm with the solution is found
                          The file is located in "solution/".
'''

import matplotlib.pyplot as plt
import shutil
import sys
import csv
import os

solFile = ["./solutions/SolGreedy#1.csv", \
           "./solutions/SolGreedy#2.csv", \
           "./solutions/SolBacktracking#1.csv", \
           "./solutions/SolBacktracking#2.csv"]

fileNumber = 6
instancesNumber = 4
res = [[ 0 for x in range(fileNumber)] for y in range(instancesNumber)]
maxRes = []
cnt = 0

for idx in solFile:
  with open(idx, newline='') as csvfile:
    print("Processing file: ", idx)
    instance = 0
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:
      if row[0] == "#":
        continue 
      x = row[0].split(",")
      res[cnt][instance] = x
      instance += 1
  cnt += 1

for idx in range(len(res[1])):
  temp = [res[0][idx][2], res[1][idx][2], res[2][idx][2] , res[3][idx][2]]
  maxN = max(temp)
  index = temp.index(max(temp))
  if(index == 0):
    alg = "GR#1"
  elif(index == 1):
    alg = "GR#2"
  elif(index == 2):
    alg = "BT#1"
  elif(index == 3):
    alg = "BT#1"
  maxRes.append([res[1][idx][0], res[1][idx][1], maxN, res[index][idx][3], alg])

# print(maxRes)

path = './solutions/bestSolutions.csv'
with open(path, mode='w') as csv_file:
  fieldnames = ['instance', 'chaoOpt', 'result', "percOnOpt", 'algorithm']
  writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
  writer.writeheader()
  
  for idx in range(len(maxRes)):
    writer.writerow({'instance' : maxRes[idx][0], 
                     'chaoOpt' : maxRes[idx][1],
                     'result' : maxRes[idx][2],
                     'percOnOpt' : maxRes[idx][3],
                     'algorithm' : maxRes[idx][4]})

solPath = "./outputs/routeHops/bestRoutes/"
if(not os.path.isdir(solPath)):
  os.mkdir("./outputs/routeHops/bestRoutes/")

for idx in range(len(maxRes)):
  fileName = maxRes[idx][0].split(".txt")
  
  if(maxRes[idx][4] == "BT#1"):
    pathFrom = "./outputs/routeHops/backtracking/#1/" +  fileName[0] + ".out"
  elif(maxRes[idx][4] == "BT#2"):
    pathFrom = "./outputs/routeHops/backtracking/#2/" +  fileName[0] + ".out"
  elif(maxRes[idx][4] == "GR#1"):
    pathFrom = "./outputs/routeHops/greedy/#1/" +  fileName[0] + ".out"
  elif(maxRes[idx][4] == "GR#2"):
    pathFrom = "./outputs/routeHops/greedy/#2/" +  fileName[0] + ".out"
  shutil.copy(pathFrom, solPath)