from matplotlib import pyplot as plt
import pandas as pd
import seaborn as sns

def f1(row):
  if row['profit'] <= 600:
    val = "<600"
  elif row['profit'] > 600 and row['profit'] <= 750:
    val = "601-750"
  elif row['profit'] > 750 and row['profit'] <= 900:
    val = "750-900"
  else:
    val = ">901"
  return val

points = []
x = []
y = []
z = []

with open("cross.tsv") as file: # Use file to refer to the file object
  data = file.readlines()

for row in range(len(data)):
  data[row] = data[row].split('\t')
  temp = data[row][2].split('\n')
  data[row][2] = float(temp[0])
  data[row][1] = float(data[row][1])
  data[row][0] = float(data[row][0])

for idx in range(len(data)):
  x.append(data[idx][0])
  y.append(data[idx][1])
  z.append(data[idx][2])

w = [x, y, z]
df = pd.DataFrame(w)
df = df.transpose()
df.columns = ["wTime", "maxDeviation", "profit"]
df['rangeProfit'] = df.apply(f1, axis=1)

my_dpi=96
plt.figure(figsize=(1920/my_dpi, 1080/my_dpi), dpi=my_dpi)
sns.scatterplot(data=df, x="wTime", y="maxDeviation", hue="rangeProfit", size="profit") # marker="+")
 
namefile = ("crossP")
plt.savefig(namefile + ".png")