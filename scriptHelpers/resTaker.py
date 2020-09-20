'''
  TThe function takes the output from Parallel.cpp and reorder the solutions, compare it with chao's results and
  made an ".csv" output file with all those informations.

  Usage:
    python3 ./scriptHelpers/restaker.py

  Input files:
    - solutions/[algoritm].csv : files ".csv" that contain the results of multi thread c++ executer.
                                  The file is located in "solutions" directory.

  Output files:
    - AutoGen[algorithm].csv : file with the information needed. 
                               The file is located in "solutions" directory.
'''

import csv
import sys
import os

def perGenerator(mySol, chaoOpt):
  if(chaoOpt == 0):
    return 1.0
  return mySol/chaoOpt

solToTake = sys.argv[1]
algorithmType =sys.argv[2]
version = sys.argv[3]

chaoPath = "./paramIn/chaoResults.txt"
chaoResults = [] 

with open(chaoPath, 'r') as file:
  for line in file.readlines():
    temp = line.split(" ")
    if(temp[0] == 0):
      continue
    chaoResults.append([temp[0], int(temp[1])]) # [InstanceName, int(ChaoRes)]
# print(chaoResults)

path = "./solutions/"
if(not os.path.isdir(path)):
  os.mkdir(path)

if(algorithmType == "GR"):
  print ("SolBacktracking#" + version + ".csv")
  f1 = open("./solutions/SolGreedy#" + version + ".csv", "w") 
elif(algorithmType == "BT"):
  print ("SolBacktracking#" + version + ".csv")
  f1 = open("./solutions/SolBacktracking#" + version + ".csv", "w") 
else:
  print ("SolLocalSearch.csv")
  f1 = open("./solutions/SolLocalSearch" + version + "#HC.csv", "w") 
  f2 = open("./solutions/SolLocalSearch" + version + "#SA.csv", "w") 
  f3 = open("./solutions/SolLocalSearch" + version + "#TS.csv", "w") 
  f4 = open("./solutions/SolLocalSearch" + version + "#SD.csv", "w") 


with open(solToTake, newline='') as csvfile:
  spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
  for row in spamreader:
    instanceName = row[0] + ".txt"
    algorithm = row[1]
    infoType = row[2]
    result = int(row[3])

    for idx in range(len(chaoResults)):
      if(instanceName == chaoResults[idx][0]):
        res = result
        break
      else:
        continue
      
    chaoOpt = chaoResults[idx][1]

    stringToWrite = "{},{},{},{}\n".format(instanceName, chaoOpt, res, perGenerator(result, chaoOpt))
    if(algorithmType == "GR"):
      f1.writelines(stringToWrite) 
    elif(algorithmType == "BT"):
      f1.writelines(stringToWrite) 
    else:
      if(algorithm == "HC"):
        f1.writelines(stringToWrite) 
      elif(algorithm == "SA"):
        f2.writelines(stringToWrite) 
      elif(algorithm == "TS"):
        f3.writelines(stringToWrite) 
      elif(algorithm == "SD"):
        f4.writelines(stringToWrite) 

f1.close()
if(algorithmType == "LS"):
  f2.close()
  f3.close()
  f4.close()
          
          
      