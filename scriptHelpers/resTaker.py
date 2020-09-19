import csv
import sys
import os

def perGenerator(mySol, chaoOpt):
  if(chaoOpt == 0):
    return 1
  return mySol/chaoOpt

solToTake = ["./solutions/SolGreedy#1.csv", \
            "./solutions/SolGreedy#2.csv", \
            "./solutions/SolBacktracking#1.csv", \
            "./solutions/SolBacktracking#2.csv", \
            "./solutions/SolLocalSearchGB#SA.csv", \
            "./solutions/SolLocalSearchGB#HC.csv", \
            "./solutions/SolLocalSearchGB#SD.csv", \
            "./solutions/SolLocalSearchGB#TS.csv", \
            "./solutions/SolLocalSearchLS#SA.csv", \
            "./solutions/SolLocalSearchLS#HC.csv", \
            "./solutions/SolLocalSearchLS#SD.csv", \
            "./solutions/SolLocalSearchLS#TS.csv"]

chaoPath = "./paramIn/chaoResults.txt"

instancesNumber = 387
allSolutions = [[0 for x in range(instancesNumber)] for y in range(len(solToTake))]
chaoResults = [] 
cnt = 0

for idx in solToTake:
  with open(idx, newline='') as csvfile:
    print("Processing file: ", idx)
    instance = 0
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:
      if row[0] == "#":
        continue 
      x = row[0].split(",")
      inst = x[0].split('"')
      if(len(inst) == 1):
        allSolutions[cnt][instance] = [inst[0], int(x[1])] 
      else:
        allSolutions[cnt][instance] = [inst[1], int(x[1])] 
      instance += 1
  cnt += 1

# print(allSolutions)
# allSolutions : colonne = file, righe = istanza, contenuto della cella = riga del file csv da convertire

with open(chaoPath, 'r') as file:
  for line in file.readlines():
    temp = line.split(" ")
    if(temp[0] == 0):
      continue
    chaoResults.append([temp[0], int(temp[1])]) # [InstanceName, int(ChaoRes)]
# print(chaoResults, len(chaoResults))

path = "./solutions/"
if(not os.path.isdir(path)):
  os.mkdir(path)

for i in range(len(solToTake)):
  temp = solToTake[i].split("/")
  nameFileToPrint = temp[2]
  print(nameFileToPrint)
  with open(path + "AutoGen" + nameFileToPrint, mode='w') as csv_file:
    fieldnames = ['# instance', 'chaoOpt', 'result', "percOnOpt"]
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    writer.writeheader()
    
    for j in range(len(chaoResults)): 
      for idx in range(len(allSolutions)):
        # print(type(allSolutions[idx][i][0]), " ", type(chaoResults[j][0]))
        if(allSolutions[idx][i][0] == chaoResults[j][0]):
          print("ok")
          mySol = allSolutions[idx][i][1]
          instance = allSolutions[idx][i][0]
          chaoOpt = chaoResults[j][1]
          break
      writer.writerow({'# instance' : instance, 
                        'chaoOpt' : chaoOpt,
                        'result' : mySol,
                        'percOnOpt' : perGenerator(mySol, chaoOpt)})
      