#!/bin/env python
import numpy as np
import sys

if len(sys.argv) < 3:
   print("Insufficient number of parameters!")
   exit(-1)
   
size = int(sys.argv[1])
phase = int(sys.argv[2])

if phase == 1:
   mat = np.load('input.bin')

   mat[:,:-1].dump('output_base.bin')

   for i in range(size):
      np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])).dump('output_%s.bin' % i)
      print(np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])))
      
elif phase == 2:
   id = sys.argv[3]
   print(np.load('start/output_%s.bin' % id))
   np.linalg.det(np.load('start/output_%s.bin' % id)).dump('det_%s.bin' % id)
   print("determinant %s" % np.linalg.det(np.load('start/output_%s.bin' % id)))
   
elif phase == 3:
   result = np.empty((size, 1), dtype='float32')
   
   baseDet = np.load('work_{}/det_{}.bin'.format(*('base',)*2))
   
   for i in range(size):
      result[i,0] = np.load('work_{}/det_{}.bin'.format(*(i,)*2))
   
   result = result / baseDet
   
   print(result)
   result.dump('result.bin')
   
else:
   print("Unknown phase %d" % phase)
