#!/bin/bash
# To take the best solution on which run the Local Search #
python3 ./scriptHelpers/bestTaker.py GB
# To take the best solution about the Local Search algorithms #
python3 ./scriptHelpers/bestTaker.py LS1
python3 ./scriptHelpers/bestTaker.py LS2
# To plot some analysis about the performance of the algorithms #
python3 ./scriptHelpers/solAnalysis.py GB
python3 ./scriptHelpers/solAnalysis.py LS1
python3 ./scriptHelpers/solAnalysis.py LS2
