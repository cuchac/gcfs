#!/bin/env python3
import numpy as np
import sys
import os
import os.path
import shutil

if len(sys.argv) < 2:
   print("Insufficient number of parameters!")
   exit(-1)
   
size = sys.argv[1]
phase = int(sys.argv[2])

if phase == 1:
   # Generate base and N matrixes
   mat = np.load('input.bin')

   mat[:,:-1].dump('output_{}.bin'.format(size))

   for i in range(size):
      np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])).dump('output_{}.bin'.format(i))
      print(np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])))
      
elif phase == 2:
   # Calculate determinant
   id = sys.argv[3]

   np.linalg.det(np.load('start/output_{}.bin'.format(id))).dump('det.bin'.format(id))
   print("determinant {}".format(np.linalg.det(np.load('start/output_{}.bin'.format(id)))))
   
elif phase == 3: 
   # Collect determinants
   result = np.empty((size, 1), dtype='float32')
   
   baseDet = np.load('work_{}/det.bin'.format(size))
   
   for i in range(size):
      result[i,0] = np.load('work_{}/det.bin'.format(i))
   
   result = result / baseDet
   
   result.dump('result.bin')
   print(result)
   
else:
   # Submission using GCFS
   inputData = sys.argv[1]
   mat = np.load('input.bin')
   size = mat.shape[0]
   
   binaryPath = os.path.realpath(sys.argv[0])
   jobPath = "/mnt/gcfs/linSyst/"
   
   def conf(file, content): 
      with open(file, 'a') as f:
         f.write(content+"\n")
   
   os.mkdir(jobPath)
   
   # Phase 1
   workDir = jobPath+"start/"
   os.mkdir(workDir)
   shutil.copy(binaryPath, workDir+"data/executable")
   shutil.copy(inputData, workDir+"data/input.bin")
   conf(workDir+"config/arguments", "{} 1".format(size))
   
   # Phase 2
   for i in range(size+1):
      workDir = jobPath+"work_{}/".format(i)
      os.mkdir(workDir)
      shutil.copy(binaryPath, workDir+"data/executable")
      conf(workDir+"config/arguments", "{} 2 {}".format(size, i))
      conf(workDir+"config/depends_on", "start")
   
   # Phase 3
   shutil.copy(binaryPath, jobPath+"data/executable")
   conf(jobPath+"config/arguments", "{} 3".format(size))
   for i in range(size+1):
      conf(jobPath+"config/depends_on", "work_{}".format(i))
   
   # Submit
   # conf(jobPath+"control", "start_and_wait")
   