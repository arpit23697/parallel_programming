import matplotlib.pyplot as plt 

# opening the file and reading in the data points
fhandle = open ("plt.txt" , "r")

lines = fhandle.readlines()
title = lines[0]
x_label = lines[1]
y_label = lines[2]

plt.title(title)
plt.xlabel(x_label)
plt.ylabel(y_label)

x=3
while x < len(lines):
    legend = lines[x]
    data = lines[x+1].strip().split()
    data = [float(x) for x in data]
    plt.plot(data , label = legend)
    x+=2
plt.legend()
plt.show()