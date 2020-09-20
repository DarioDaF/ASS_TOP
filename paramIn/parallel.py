import json
import numpy as np

algos = []

for b in np.arange(0, 1.5, .1):
  for c in np.arange(0, 6, .01):
    algos.append({ "type": "greedy", "descr": f"{b}#{c}", "options": { "wTime": b, "maxDev": c } })

with open('parallel.json', 'w') as fp:
  json.dump([ { "names": ".*", "algos": algos } ], fp)
