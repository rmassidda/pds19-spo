#! /bin/env python3
import math
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import numpy as np
import sys

marker = [ '-', ':', '-^', ':^' ]

class Experiment:
    def __init__(self, name):
        self.times = []
        self.name = name

    def efficiency ( self ) :
        ef = [1]
        ef = ef + [self.times[0]/(i*self.times[i]) for i in range(len(self.times))[1:]];
        self.ef = np.array ( ef )

    def scalability(self):
        self.sc = np.array ( list ( map ( lambda t: self.times[1]/t, self.times ) ) )
        self.sc[0] = 0

    def speedup(self):
        self.sp = np.array ( list ( map ( lambda t: self.times[0]/t, self.times ) ) )
        self.sp[0] = 0

    def compute ( self ):
        self.scalability()
        self.speedup()
        self.efficiency()

def plot ( name, exp ):
    plt.figure(num=None, figsize=(12,9))

    # Maximum value
    max_x = 0
    max_y = 0
        
    index = 0
    # Add the experimental results
    for e in exp:
        # Get the required data
        if name == "Scalability":
            arr = e.sc
        elif name == "Speedup":
            arr = e.sp
        elif name == "Efficiency":
            arr = e.ef
        else:
            arr = None
        x = np.array ( range ( len ( arr ) ) )
        m_on = [i for i in range(len(x)) if i%10==0]
        plt.plot( x, arr, marker[index], color='black', label=e.name, markevery=m_on )
        for t in arr:
            max_y = max ( t, max_y )
        max_x = max ( max_x, len ( arr ) )
        index = index + 1

    if name != "Efficiency":
        max_y = math.ceil ( max_y )
        arr = np.array ( range ( max_y + 1) )
        plt.plot( arr, arr, color='black' )
    else:
        plt.plot ( range(max_x), [1 for i in range(max_x)], color='black' )

    plt.xlabel('workers')
    plt.title(name)
    plt.legend()
    plt.savefig(name+'.png', bbox_inches="tight" )

# Experimental results
exp = []

# Read experiments
for file_path in sys.argv[1:]:
    with open(file_path) as file:
        e = Experiment ( file_path[:-4] )
        for line in file:
            parts = line.split()
            e.times.append ( int ( parts[3] ) )
        exp.append ( e )

# Compute the metrics
for e in exp:
    e.compute()

plot ( "Scalability", exp )
plot ( "Speedup", exp )
plot ( "Efficiency", exp )
