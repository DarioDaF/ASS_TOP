'''
  The function plots for each parameter, some information about it. The plot is made for all the maps (typing 0 
  in the command line) or to one specified map (typing its number).
  The informations plotted are:
    - Profit : mean profit of all instances (percentage on Chao's best);
    - minElem : lowest result profit found (percentage on Chao's best);
    - maxElem : highest result profit found (percentage on Chao's best);
    - devStd : standard deviation of all the instances. It is used to print Profit +/- devStd;

  Usage:
    python3 ./scriptHelpers/plotParams.py [map] [parameter]

  Input files:
    - solPar[param].csv : file ".csv" that contain the value to plot. The program parse all the files.
                          The file is located in "outputs/greedy/plots" directory or outputs/greedy/plots/Map[param].

  Output files:
    - solPar[param].png : image file with the plot. 
                          The file is located in "outputs/greedy/plots" directory or outputs/greedy/plots/Map[param].
'''

import matplotlib.pyplot as plt
import sys

if(sys.argv[1] == '0'): 
  # Process all the instances
  # For all the files to print
  files = ['./outputs/greedy/plots/solParwProfit.csv', \
           './outputs/greedy/plots/solParwTime.csv', \
           './outputs/greedy/plots/solParmaxDeviation.csv', \
           './outputs/greedy/plots/solParwNonCost.csv']
  
  print("Processing all Maps")
  for file in files:
    # Check the correct instance
    x = file.split('/') 
    y = x[4].split('Par')
    z = y[1].split('.')
    if(z[0] != sys.argv[2]):
      continue

    print("Processign file: " + file)
    profit = []
    minElem = []
    maxElem = []
    devStdUp = []
    devStdDown = []
    param = []
    l = []

    # Read the input ".csv" file 
    with open(file, 'r', encoding='utf-8') as infile: 
      for line in infile:
        l = line.split(",")
        param.append(float(l[0]))
        profit.append(float(l[1]))
        minElem.append(float(l[2])) 
        maxElem.append(float(l[3]))
        devStdUp.append(float(l[1]) + float(l[4]))
        devStdDown.append(float(l[1]) - float(l[4]))
    
    # Print the output scaled, with all the informations
    plt.style.use('seaborn-darkgrid')
    my_dpi=96
    plt.figure(figsize=(480/my_dpi, 480/my_dpi), dpi=my_dpi)
        
    plt.plot(param, profit)
    plt.plot(param, minElem)
    plt.plot(param, maxElem)
    plt.plot(param, devStdUp)
    plt.plot(param, devStdDown)
        
    # Change xlim
    plt.xlim(min(param), max(param))
        
    # Add titles and legend
    plt.legend(["profit", "minElem", "maxElem", "Prf+dStd", "Prf-dStd"])

    # For all the maps print axes and labels and titles 
    if(file == './outputs/greedy/plots/solParwProfit.csv'):
      plt.title("a.Plot wProfit params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wProfit")
    elif(file == './outputs/greedy/plots/solParwTime.csv'):
      plt.title("b.Plot wTime params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wTime")
    elif(file == './outputs/greedy/plots/solParmaxDeviation.csv'):
      plt.title("c.Plot maxDeviation params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param maxDeviation")
    elif(file == './outputs/greedy/plots/solParwNonCost.csv'):
      plt.title("d.Plot wNonCost params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wNonCost")

    plt.ylabel("Profit, minElem, maxElem, Prf+dStd, Prf-dStd")
        
    #Save file
    namefile = file.split(".csv")
    plt.savefig(namefile[0] + ".png")

else:
  # Process one particular map
  # For all the files to print
  files = ['./outputs/greedy/plots/Map' + sys.argv[1] + '/solParwProfit.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParwTime.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParmaxDeviation.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParwNonCost.csv']
  
  print("Processing Map " + sys.argv[1])
  # Check the correct instance
  for file in files: 
    x = file.split('/') 
    y = x[5].split('Par')
    z = y[1].split('.')
    if(z[0] != sys.argv[2]):
      continue
        
    print("Processign file: " + file) 
    profit = []
    minElem = []
    maxElem = []
    devStdUp = []
    devStdDown = []
    param = []
    l = []

    # Read the input ".csv" file 
    with open(file, 'r', encoding='utf-8') as infile: 
      for line in infile:
        l = line.split(",")
        param.append(float(l[0]))
        profit.append(float(l[1]))
        minElem.append(float(l[2])) 
        maxElem.append(float(l[3]))
        devStdUp.append(float(l[1]) + float(l[4]))
        devStdDown.append(float(l[1]) - float(l[4]))
           
    # Print the output scaled, with all the informations
    plt.style.use('seaborn-darkgrid')
    my_dpi=96
    plt.figure(figsize=(480/my_dpi, 480/my_dpi), dpi=my_dpi)
       
    plt.plot(param, profit)
    plt.plot(param, minElem)
    plt.plot(param, maxElem)
    plt.plot(param, devStdUp)
    plt.plot(param, devStdDown)
            
    # Change xlim
    plt.xlim(min(param), max(param))
            
    # Add titles and legend
    plt.legend(["profit", "minElem", "maxElem", "devStd", "Prf+dStd", "Prf-dStd"])

    # For all the maps print axes and labels and titles
    if(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParwProfit.csv'):
      plt.title("a.Plot wProfit params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wProfit")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParwTime.csv'):
      plt.title("b.Plot wTime params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wTime")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParmaxDeviation.csv'):
      plt.title("c.Plot maxDeviation params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param maxDeviation")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParwNonCost.csv'):
      plt.title("d.Plot wNonCost params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("param wNonCost")

    plt.ylabel("Profit, minElem, maxElem, Prf+dStd, Prf-dStd")
            
    #Save file
    namefile = file.split(".csv")
    plt.savefig(namefile[0] + ".png")

