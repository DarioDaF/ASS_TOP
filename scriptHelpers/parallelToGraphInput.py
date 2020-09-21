
from csv import DictReader
from collections import defaultdict, Counter
from statistics import fmean, stdev
from os import path, makedirs, chdir
from shutil import copyfile

scriptPath = path.dirname(path.abspath(__file__))
chdir(scriptPath)

def indexOrDefault(arr, val, default):
  if val in arr:
    return arr.index(val)
  else:
    return default

def indexOrLen(arr, val):
  return indexOrDefault(arr, val, len(arr))

ROOT_PATH = '..'
OUTPUTS_PATH = ROOT_PATH + '/outputs/parallel'
MAP_PATH = OUTPUTS_PATH + '/csv'
BESTROUTES_PREFIS = ROOT_PATH + '/outputs/routeHops/bestRoutes'
CHAO_FILE = ROOT_PATH + '/paramIn/chaoResults.txt'
PRIORITY = [
  'GREEDY (default)', 'GREEDY RANGE (default)',
  'BT (default)', 'BT FOCUS (default)', 'BT (with drop)', 'BT FOCUS (with drop)',
  'SD (default)', 'HC (default)', 'SA (default)', 'TS (default)', 'TS (pumped30000)'
]

COPY_BEST = False
LOAD_LOCAL = True

# Our maps' results list
filesMapResults = ['PARALLEL_20200921_GREEDY.csv', 'PARALLEL_20200921_BT_NO_FOCUS.csv', 'PARALLEL_20200921_BT_FOCUS.csv']
if LOAD_LOCAL:
  filesMapResults.extend(['PARALLEL_20200921_LOCAL.csv', 'PARALLEL_20200921_TS_PUMPED.csv'])
filesMapResults = [ MAP_PATH + '/' + fileMapResult for fileMapResult in filesMapResults ]

mapOur = defaultdict(list)
mapChao = {}

# Load our maps' results
for fileMapResult in filesMapResults:
  with open(fileMapResult, 'r') as fp:
    r = DictReader(fp, fieldnames=('name', 'algo', 'descr', 'profit', 'feasible'))
    for row in r:
      row['feasible'] = bool(int(row['feasible']))
      row['profit'] = int(row['profit'])
      if not row['feasible']:
        row['profit'] = 0
      mapOur[row['name']].append(row)

# Load chao's results
with open(CHAO_FILE, 'r') as fp:
  r = DictReader(fp, fieldnames=('name', 'profit'), delimiter=' ')
  for row in r:
    row['profit'] = int(row['profit'])
    mapChao[path.splitext(row['name'])[0]] = ({ 'algo': 'chao', **row })

# Get list of common names
names = set(mapOur.keys()) & set(mapChao.keys())

# Create full name for algos
for name in names:
  for algo in mapOur[name]:
    algo['fullAlgo'] = f"{algo['algo']} ({algo['descr']})"

# Get list of algo names
algoNames = set(algo['fullAlgo'] for name in names for algo in mapOur[name])

# Compute percentual profit
for name in names:
  bestProfit = mapChao[name]['profit']
  for algo in mapOur[name]:
    if mapChao[name]['profit'] > 0:
      algo['percProfit'] = algo['profit'] / bestProfit
      if algo['percProfit'] > 1:
        print(f"WARINIG: {algo['fullAlgo']} on {algo['name']} had profit of {algo['profit']} / {bestProfit}")
    else:
      algo['percProfit'] = 1


# Formato:
#   nome, chaoProfit, profit, percProfit

TRANSLATE = {
  'BT (with drop)': 'SolBacktracking#1.csv',
  'BT FOCUS (with drop)': 'SolBacktracking#2.csv',
  'GREEDY (default)': 'SolGreedy#1.csv',
  'GREEDY RANGE (default)': 'SolGreedy#2.csv',
  'SA (default)': 'SolLocalSearchGB#SA.csv',
  'SD (default)': 'SolLocalSearchGB#SD.csv',
  'HC (default)': 'SolLocalSearchGB#HC.csv',
  'TS (default)': 'SolLocalSearchGB#TS.csv',
  'TS (pumped30000)': 'SolLocalSearchGB#TS_pumped.csv'
}

for algoName in TRANSLATE:
  fname = TRANSLATE[algoName]
  TRANSLATE[algoName] = { 'name': fname, 'fp': open(fname, 'w') }

listNames = list(names)
listNames.sort()
for name in listNames:
  for algo in mapOur[name]:
    if algo['fullAlgo'] in TRANSLATE:
      fp = TRANSLATE[algo['fullAlgo']]['fp']
      fp.write(f"{algo['name']},{mapChao[name]['profit']},{algo['profit']},{algo['percProfit']}\n")

for algoName in TRANSLATE:
  TRANSLATE[algoName]['fp'].close()
