#! /bin/env python3
import math
import matplotlib.pyplot as plt
import numpy as np
import sys

class Experiment:
    def __init__(self):
        self.times = []

    def setSeq ( self, seq ):
        self.seq = seq

    def scalability(self):
        self.sc = np.array ( [0] + list ( map ( lambda t: self.times[0]/t, self.times ) ) )
        return self.sc

    def speedup(self):
        self.sp = np.array ( [0] + list ( map ( lambda t: self.seq/t, self.times ) ) )
        return self.sp

def createPlot ( name, x, ff, cpp ):
    plt.clf ()
    plt.figure(figsize=(15, 10))
    plt.plot(x, x, color='black', label='Linear')
    plt.plot(x, ff, 's--', color='black', label='Fast Flow')
    plt.plot(x, cpp, 'o:', color='black', label='C++ Thread')
    plt.xlabel('workers')
    plt.ylabel('performance')
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

# Add the best sequential time
fast_flow.setSeq ( seq )
cpp_thread.setSeq ( seq )

x = np.array ( x )
createPlot ( "Scalability", x, fast_flow.scalability(), cpp_thread.scalability() )
createPlot ( "Speedup", x, fast_flow.speedup(), cpp_thread.speedup() )

# print ( x )
# print ( fast_flow.scalability() )
# print ( fast_flow.speedup() )
# print ( cpp_thread.scalability() )
# print ( cpp_thread.speedup() )
