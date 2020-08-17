#!/usr/bin/env python3

import imageio
import os
import shutil
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from time import sleep
from mpl_toolkits.mplot3d import axes3d, Axes3D
#from matplotlib.animation import FuncAnimation
import matplotlib.patches as patches

access_rights = 0o777

try:
    os.mkdir('data', access_rights)
except FileExistsError:
    shutil.rmtree('data')
    os.mkdir('data', access_rights)

ccolor=['k','r','g']

with open("output.dat") as f:
    data = f.readlines()

iterations, npeople, x_As, x_Bs, y_As, y_Bs,z_As, z_Bs, r_circle = data[0].split("\t")
x_A = float(x_As)
x_B = float(x_Bs)
y_A = float(y_As)
y_B = float(y_Bs)
z_A = float(z_As)
z_B = float(z_Bs)
r_circle=float(r_circle)
skip = 0

iterations = iterations[1:]

npeople = int(npeople)
iterations = int(iterations)

X = [[] for x in range(npeople)]
Y = [[] for x in range(npeople)]
Z = [[] for x in range(npeople)]
C = [[] for x in range(npeople)]


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

iterator = 0
new_sick_vector = []
bar_x=[]
labels = ['Voyager 1', 'Sol', 'Tierra', 'Marte', 'Jupiter', 'Saturno']
area = [1, 100, 20, 15, 60, 30] #km

images = []

for line in data[1:]:
    if "#" in line[0]:
        ax.cla()
        ax.set_xlim3d(x_A,x_B)
        ax.set_ylim3d(y_A,y_B)
        ax.set_zlim3d(z_A,z_B)
        iteration=int(line[1:])
        iterator = 0
    else:
        mass, position, velocity = line.split("\t")
        position = position[1:-1]
        #print(position)
        mass=float(mass)
        x,y,z = position.split(",")
        x = float(x)
        y = float(y)
        z = float(z)
        ax.scatter(x, y, z, s=area[iterator], marker='o')
        
        iterator = iterator + 1

    if (iterator == npeople):
        plt.draw()
        plt.legend(labels[:npeople], loc='upper left')
        fig.savefig("data/"+("%08d"%iteration)+"-view3D.png")
        images.append(imageio.imread("data/"+("%08d"%iteration)+"-view3D.png"))

imageio.mimsave('data/voyager.gif', images)
