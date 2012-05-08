#!/bin/env python3
import numpy as np
import sys
import os
import os.path
import shutil

if len(sys.argv) < 2:
   print("Insufficient number of parameters!")
   exit(-1)
   
phase = int(sys.argv[1])

if phase == 1:
   # Generate base and N matrixes
   mat = np.load(sys.argv[2])

   mat[:,:-1].dump('matrix_{:05d}.bin'.format(0))

   for i in range(mat.shape[0]):
      np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])).dump('matrix_{:05d}.bin'.format(i+1))
      print("Matrix: ", i, "\n", np.hstack((mat[:,:i], mat[:,-1], mat[:,i+1:-1])))
      
elif phase == 2:
   # Calculate determinant
   inputMatrix = sys.argv[2]

   np.linalg.det(np.load(inputMatrix)).dump('det_{}'.format(os.path.basename(inputMatrix)))
   print("determinant {}".format(np.linalg.det(np.load(inputMatrix))))
   
elif phase == 3: 
   # Collect determinants
   size = len(sys.argv)-3;
   result = np.empty((size, 1), dtype='float32')
   
   baseDet = np.load(sys.argv[2])
   
   for i in range(size):
      result[i,0] = np.load(sys.argv[i+3])
   
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
   