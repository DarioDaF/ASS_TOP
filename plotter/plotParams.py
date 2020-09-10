'''
  The function plots for ach parametere, some information about it. The plot is made for all the maps (typing 0 
  in the command line) or to one specified map (typing its number).
  The informations plotted are:
    - Profit : mean profit of all instances (percentage on Chao's best);
    - minElem : lowest result profit found (percentage on Chao's best);
    - maxElem : highest result profit found (percentage on Chao's best);
    - devStd : standard deviation of all the instances. It is used to print Profit +/- devStd;

  Usage:
    python3 ./plotter/plotParams.py [map] [parameter]

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
  files = ['./outputs/greedy/plots/solParA.csv', \
           './outputs/greedy/plots/solParB.csv', \
           './outputs/greedy/plots/solParC.csv', \
           './outputs/greedy/plots/solParD.csv']

  for file in files:
    # Check the correct instance
    x = file.split('.') 
    mapLetter = x[1][-1]
    if(mapLetter != sys.argv[2]):
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
    if(file == './outputs/greedy/plots/solParA.csv'):
      plt.title("Plot A params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramA")
    elif(file == './outputs/greedy/plots/solParB.csv'):
      plt.title("Plot B params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramB")
    elif(file == './outputs/greedy/plots/solParC.csv'):
      plt.title("Plot C params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramC")
    elif(file == './outputs/greedy/plots/solParD.csv'):
      plt.title("Plot D params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramD")

    plt.ylabel("Profit, minElem, maxElem, Prf+dStd, Prf-dStd")
        
    #Save file
    namefile = file.split(".csv")
    plt.savefig(namefile[0] + ".png")

else:
  # Process one particular map
  # For all the files to print
  files = ['./outputs/greedy/plots/Map' + sys.argv[1] + '/solParA.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParB.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParC.csv', \
           './outputs/greedy/plots/Map' + sys.argv[1] + '/solParD.csv']
  
  # Check the correct instance
  for file in files: 
    x = file.split('.')
    mapLetter = x[1][-1]
    if(mapLetter != sys.argv[2]):
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
    if(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParA.csv'):
      plt.title("Plot A params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramA")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParB.csv'):
      plt.title("Plot B params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramB")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParC.csv'):
      plt.title("Plot C params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramC")
    elif(file == './outputs/greedy/plots/Map' + sys.argv[1] + '/solParD.csv'):
      plt.title("Plot D params", loc='left', fontsize=12, fontweight=0, color='orange')
      plt.xlabel("paramD")

    plt.ylabel("Profit, minElem, maxElem, Prf+dStd, Prf-dStd")
            
    #Save file
    namefile = file.split(".csv")
    plt.savefig(namefile[0] + ".png")

