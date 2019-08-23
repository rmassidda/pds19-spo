#! /bin/env python3
import math
import matplotlib.pyplot as plt
import numpy as np
import sys

class Experiment:
    def __init__(self):
        self.times = []

    def efficiency ( self ) :
        self.ef = np.array ( [1] + [self.seq/((i+1)*self.times[i]) for i in range(len(self.times))])

    def scalability(self):
        self.sc = np.array ( [0] + list ( map ( lambda t: self.times[0]/t, self.times ) ) )

    def speedup(self):
        self.sp = np.array ( [0] + list ( map ( lambda t: self.seq/t, self.times ) ) )

    def computePerformance ( self, seq ):
        self.seq = seq
        self.scalability()
        self.speedup()
        self.efficiency()

def createPlot ( name, x, ff, cpp, linear=True ):
    plt.clf ()
    if linear:
        plt.figure(figsize=(30, 30))
        plt.plot(x, x, color='black', label='Linear')
    else:
        plt.figure(figsize=(30,10))
        plt.plot ( x, [1 for i in range(len(x))], color='black' )
        
    plt.plot(x, ff, 'o:', color='black', label='Fast Flow')
    plt.plot(x, cpp, 's--', color='black', label='C++ Thread')
    plt.xlabel('workers')

    # if linear:
    #     plt.ylim([0,30])

    plt.title(name)
    plt.legend()
    plt.savefig(name+'.png', bbox_inches="tight" )
    # plt.show()

# Experimental results
fast_flow = Experiment ()
cpp_thread = Experiment ()

x = [0]
i = 1
seq = math.inf
for line in sys.stdin:
    parts = line.split()
    if parts[0] == 'sequential':
        seq = min ( seq, int ( parts[3] ) )
    elif parts[0] == 'mapreduce':
        cpp_thread.times.append ( int ( parts[3] ) )
        x.append ( i )
        i = i + 1
    elif parts[0] == 'fastflow':
        fast_flow.times.append ( int ( parts[3] ) )

# Compute the performance metrics
fast_flow.computePerformance(seq)
cpp_thread.computePerformance(seq)

x = np.array ( x )
createPlot ( "Scalability", x, fast_flow.sc, cpp_thread.sc )
createPlot ( "Speedup", x, fast_flow.sp, cpp_thread.sp )
createPlot ( "Efficiency", x, fast_flow.ef, cpp_thread.ef, linear=False )

# print ( len ( fast_flow.times ) )
# print ( len ( fast_flow.sc ) )
# print ( len ( fast_flow.ef ) )
# print ( len ( x )  )
# print ( fast_flow.ef )
# print ( fast_flow.sc )
# print ( fast_flow.sp )
# print ( cpp_thread.ef )
# print ( cpp_thread.sc )
# print ( cpp_thread.sp )
