
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

# Get best for each map
bestAlgo = {}
bestHistoRep = defaultdict(int)
bestHistoAbs = defaultdict(int)
for name in names:
  # Sort mapOur[name] to have greedy first, then bt to get correct best (for output -> local)
  _mapOur = mapOur[name].copy()
  _mapOur.sort(key=lambda algo: indexOrLen(PRIORITY, algo['fullAlgo']))
  best = max(_mapOur, key=lambda algo: algo['profit'])
  bestAlgo[name] = best
  bestProfit = best['profit']
  n = 0
  for algo in _mapOur:
    if algo['profit'] >= bestProfit:
      n += 1
      bestHistoRep[algo['fullAlgo']] += 1
  if n == 1:
    bestHistoAbs[best['fullAlgo']] += 1

# bestHisto is best with PRIORITY
bestHisto = Counter(map(lambda algo: algo['fullAlgo'], bestAlgo.values()))
_histoKeys = list(bestHisto)
_histoKeys.sort(key=lambda algoName: indexOrLen(PRIORITY, algoName))
for algoName in _histoKeys:
  print(f"{algoName :30s} priority solved: {bestHisto[algoName] :3d} instances")
print()

for algoName in algoNames:
  percProfits = [ algo['percProfit'] for name in names for algo in mapOur[name] if algo['fullAlgo'] == algoName ]
  meanPercProfits = fmean(percProfits)
  stdevPercProfits = stdev(percProfits)
  minPercProfits = min(percProfits)
  maxPercProfits = max(percProfits)
  print(f"{algoName :30s} -> {meanPercProfits * 100 :6.2f}% (sigma: {stdevPercProfits * 100 :6.2f}%, min: {minPercProfits * 100 :6.2f}%, max: {maxPercProfits * 100 :6.2f}%, best in {bestHistoRep[algoName] :3d} instances, absolute in {bestHistoAbs[algoName] :3d})")
print()

if COPY_BEST:
  makedirs(BESTROUTES_PREFIS, exist_ok=True)
  for name in bestAlgo:
    algo = bestAlgo[name]
    copyfile(f"{OUTPUTS_PATH}/{algo['algo']}/{algo['descr']}/{algo['name']}.out", f"{BESTROUTES_PREFIS}/{algo['name']}.out")
