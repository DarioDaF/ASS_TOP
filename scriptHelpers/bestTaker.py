'''
  The function evaluate the best solution obtained from all the algorithms and take the best algorithm. Than 
  save the best path for Local Search algortim. Also Write a new ".csv" file with the best solutions found and 
  other useful informations.

  Usage:
    python3 ./scriptHelpers/bestTaker.py [bestVersion]

    bestVersion: 
        - GB : Greedy and Backtracking
        - LS : Local Search from GB
        - LS2 : Local Search from LS1

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

def printAlg(n, idx, res, maxRes):
  if(float(maxRes[idx][2]) == 0.0): 
    return 0.0 
  else: 
    return 1 - float(res[n][idx][2]) / float(maxRes[idx][2])

ver = sys.argv[1]
ver = ver.strip()
print("Taking best version ", ver)

if(ver == 'GB'): 
  solFile = ["./solutions/SolGreedy#1.csv", \
             "./solutions/SolGreedy#2.csv", \
             "./solutions/SolBacktracking#1.csv", \
             "./solutions/SolBacktracking#2.csv"]
elif(ver == 'LS1'):
  solFile = ["./solutions/SolLocalSearchGB#SA.csv", \
             "./solutions/SolLocalSearchGB#HC.csv", \
             "./solutions/SolLocalSearchGB#SD.csv", \
             "./solutions/SolLocalSearchGB#TS.csv"]
else:
  solFile = ["./solutions/SolLocalSearchLS#SA.csv", \
             "./solutions/SolLocalSearchLS#HC.csv", \
             "./solutions/SolLocalSearchLS#SD.csv", \
             "./solutions/SolLocalSearchLS#TS.csv"]

instancesNumber = 387
fileNumber = 4
res = [[ 0 for x in range(instancesNumber)] for y in range(fileNumber)]
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

for idx in range(len(res[0])):
  temp = [int(res[0][idx][2]), int(res[1][idx][2]), int(res[2][idx][2]) , int(res[3][idx][2])]
  maxN = max(temp)
  index = temp.index(max(temp))

  if(ver == 'GB'): 
    if(index == 0):
      alg = "GR#1"
    elif(index == 1):
      alg = "GR#2"
    elif(index == 2):
      alg = "BT#1"
    elif(index == 3):
      alg = "BT#2"
  else:
    if(index == 0):
      alg = "SA"
    elif(index == 1):
      alg = "HC"
    elif(index == 2):
      alg = "SD"
    elif(index == 3):
      alg = "TS"
    
  # print(res[0][idx][0], ", ", temp, ", ", maxN, ", ", index, ", ", alg)
  # print(res[1][idx][0], ", ", res[1][idx][1], ", ", maxN, ", ",res[index][idx][3], ", ", alg)
  maxRes.append([res[1][idx][0], res[1][idx][1], maxN, res[index][idx][3], alg])

path = "./solutions/bestSolutions/"
if(not os.path.isdir(path)):
  os.mkdir(path)
with open(path + "bestSol" + ver + ".csv", mode='w') as csv_file:
  
  if(ver == "GB"):
    fieldnames = ['instance', 'chaoOpt', 'result', "percOnOpt", 'bestAlgorithm',"dtGR#1", "dtGR#2", "dtBT#1", "dtBT#2", "dtMedium"]
  else:
    fieldnames = ['instance', 'chaoOpt', 'result', "percOnOpt", 'bestAlgorithm', "dtSA", "dtHC", "dtSD", "dtTS", "dtMedium"]
      
  writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
  writer.writeheader()
  
  for idx in range(len(maxRes)):
    name = maxRes[idx][0].split('.txt')
    if(ver == "GB"):
      writer.writerow({'instance' : name[0] + '.txt', 
                       'chaoOpt' : maxRes[idx][1],
                       'result' : maxRes[idx][2],
                       'percOnOpt' : maxRes[idx][3],
                       'bestAlgorithm' : maxRes[idx][4],
                       'dtGR#1' : printAlg(0, idx, res, maxRes),
                       'dtGR#2' : printAlg(1, idx, res, maxRes),
                       'dtBT#1' : printAlg(2, idx, res, maxRes),
                       'dtBT#2' : printAlg(3, idx, res, maxRes),
                       'dtMedium' : (printAlg(0, idx, res, maxRes) + printAlg(1, idx, res, maxRes) + printAlg(2, idx, res, maxRes) + printAlg(3, idx, res, maxRes)) / 4})
    else:
      writer.writerow({'instance' : name[0] + '.txt', 
                       'chaoOpt' : maxRes[idx][1],
                       'result' : maxRes[idx][2],
                       'percOnOpt' : maxRes[idx][3],
                       'bestAlgorithm' : maxRes[idx][4],
                       'dtSA' : printAlg(0, idx, res, maxRes),
                       'dtHC' : printAlg(1, idx, res, maxRes),
                       'dtSD' : printAlg(2, idx, res, maxRes),
                       'dtTS' : printAlg(3, idx, res, maxRes),
                       'dtMedium' : (printAlg(0, idx, res, maxRes) + printAlg(1, idx, res, maxRes) + printAlg(2, idx, res, maxRes) + printAlg(3, idx, res, maxRes)) / 4})

if(ver == 'GB'): 
  solPath = "./outputs/routeHops/bestRoutes/GB/"
  if(not os.path.isdir(solPath)):
    os.mkdir("./outputs/routeHops/bestRoutes/GB/")

  for idx in range(len(maxRes)):
    fileName1 = maxRes[idx][0].split(".txt")
    
    if(maxRes[idx][4] == "BT#1"):
      pathFrom = "./outputs/routeHops/backtracking/#1/" +  fileName1[0] + ".out"
    elif(maxRes[idx][4] == "BT#2"):
      pathFrom = "./outputs/routeHops/backtracking/#2/" + fileName1[0] + ".out"
    elif(maxRes[idx][4] == "GR#1"):
      pathFrom = "./outputs/routeHops/greedy/#1/" +  fileName1[0] + ".out"
    elif(maxRes[idx][4] == "GR#2"):
      pathFrom = "./outputs/routeHops/greedy/#2/" + fileName1[0] + ".out"
    shutil.copy(pathFrom, solPath)
else:
  solPath = "./outputs/routeHops/bestRoutes/LS/"
  if(not os.path.isdir(solPath)):
    os.mkdir("./outputs/routeHops/bestRoutes/LS/")

  for idx in range(len(maxRes)):
    fileName1 = maxRes[idx][0].split(".txt")
    fileName2 = fileName1[0].split('"')
    
    if(maxRes[idx][4] == "SA"):
      pathFrom = "./outputs/routeHops/localsearch/GB/SA/" +  fileName2[1] + ".out"
    elif(maxRes[idx][4] == "HC"):
      pathFrom = "./outputs/routeHops/localsearch/GB/HC/" + fileName2[1] + ".out"
    elif(maxRes[idx][4] == "SD"):
      pathFrom = "./outputs/routeHops/localsearch/GB/SD/" +  fileName2[1] + ".out"
    elif(maxRes[idx][4] == "TS"):
      pathFrom = "./outputs/routeHops/localsearch/GB/TS/" + fileName2[1] + ".out"
    shutil.copy(pathFrom, solPath)
