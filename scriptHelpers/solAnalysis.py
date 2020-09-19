'''
  The function plots for all the algorithms, two figure: in the first it can be seen the trend of all the algorithms
  in respect of chao's solutions (optimum). In the second one, it is presented for each algoritm, the count of best 
  solutions found (who found the optimal solutions for an instance).

  Usage:
    python3 ./scriptHelpers/solAnalysis.py [version]
    
    version: 
        - GB : Greedy and Backtracking
        - LS : Local Search from GB
        - LS2 : Local Search from LS1

  Input files:
    - Sol[algorithm]#[version].csv : file ".csv" that contain the solution of one version of one algorithm.
                                     The file is located in "solutions/" directory.

  Output files:
    - solPlot#[version].png and solPlotCount[version] : image file with the plot. 
                                                        The file is located in "solution/resAnalysis/".
'''

import matplotlib.pyplot as plt
import csv
import sys
import os

# def minAlgo(instRes):
#   return min(instRes[0][3], instRes[1][3], instRes[2][3], instRes[3][3])
#   return (instRes[0][3] + instRes[1][3] + instRes[2][3] + instRes[3][3])

def minAlgo(instRes):
  return instRes[3][3]

ver = sys.argv[1]
ver = ver.strip()
print("Plotting analsysis version " , ver)

if(ver == 'GB'): 
  solFile = ["./solutions/SolGreedy#1.csv", \
             "./solutions/SolGreedy#2.csv", \
             "./solutions/SolBacktracking#1.csv", \
             "./solutions/SolBacktracking#2.csv"]
elif(ver == 'LS'):
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
      res[instance][cnt] = x
      instance += 1
  cnt += 1
  
resGr1 = []
resGr2 = []
resBt1 = []
resBt2 = []
instancesF = []

res.sort(reverse = True, key = minAlgo)

for idx in range(len(res)):
  instancesF.append(res[idx][1][0])
  resGr1.append(float(res[idx][0][3]) * 100)
  resGr2.append(float(res[idx][1][3]) * 100) 
  resBt1.append(float(res[idx][2][3]) * 100)
  resBt2.append(float(res[idx][3][3]) * 100)

# Print the output scaled, with all the informations
plt.style.use('seaborn-darkgrid')
my_dpi=200
plt.figure(figsize=(6000/my_dpi, 2080/my_dpi), dpi=my_dpi)

plt.plot(instancesF, resGr1)
plt.plot(instancesF, resGr2)
plt.plot(instancesF, resBt1)
plt.plot(instancesF, resBt2)

plt.xticks(rotation=90)
# Add titles and legend
if(ver == 'GB'): 
  plt.legend(["Gr#1", "Gr#2", "Bt#1", "Bt#2"])
else:
  plt.legend(["SA", "HC", "SD", "TS"])

plt.title("Algorithm's Results", loc='left', fontsize=12, fontweight=0, color='orange')
plt.xlabel("instances")
plt.ylabel("res/chaoOpt")

for idx in range(len(res)):
  temp = [float(res[idx][0][2]), float(res[idx][1][2]), float(res[idx][2][2]) , float(res[idx][3][2])]
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
  maxRes.append([res[idx][0][0], float(res[idx][1][1]), float(maxN), float(res[idx][index][3]), alg])

#Save file
namefile = "./solutions/resultAnalysis/"
if(not os.path.isdir(namefile)):
  os.mkdir(namefile)
plt.savefig(namefile + "solPlot" + ver + ".png")

# Print the output scaled, with all the informations
plt.style.use('seaborn-darkgrid')
my_dpi=96
plt.figure(figsize=(496/my_dpi, 496/my_dpi), dpi=my_dpi)

if(ver == 'GB'): 
  x = ['GR#1', 'GR#2', 'BT#1', 'BT#2']
else:
  x = ["SA", "HC", "SD", "TS"]

cntGr1 = 0
cntGr2 = 0
cntBt1 = 0
cntBt2 = 0
for i in range(len(maxRes)):
  if(ver == 'GB'): 
    if(maxRes[i][4] == "GR#1"):
      cntGr1 += 1
    elif(maxRes[i][4] == "GR#2"):
      cntGr2 += 1
    elif(maxRes[i][4] == "BT#1"):
      cntBt1 += 1
    elif(maxRes[i][4] == "BT#2"):
      cntBt2 += 1
  else:
    if(maxRes[i][4] == "SA"):
      cntGr1 += 1
    elif(maxRes[i][4] == "HC"):
      cntGr2 += 1
    elif(maxRes[i][4] == "SD"):
      cntBt1 += 1
    elif(maxRes[i][4] == "TS"):
      cntBt2 += 1

y = [cntGr1, cntGr2, cntBt1, cntBt2]

# Create barplot
plt.bar(x,y, color=['firebrick', 'green', 'blue', 'purple'])

plt.xlabel('Algorithm')
plt.ylabel("Count")
plt.title('Count of best solution for each algoritm', loc='left', fontsize=12, fontweight=0, color='orange')

namefile = "./solutions/resultAnalysis/"
if(not os.path.isdir(namefile)):
  os.mkdir(namefile)
plt.savefig(namefile + "solPlotCount" + ver + ".png")