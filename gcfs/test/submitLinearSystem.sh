#!/bin/sh -x

# Set GCFS location
GCFS="/mnt/gcfs/"
ROOT="linsyst"
EXECUTABLE="linearSystem.py"
DAG=$GCFS/$ROOT
N=11

# Starting and collecting tasks
mkdir $DAG
mkdir $DAG/start

# Copy executables and set arguments
cp $EXECUTABLE $DAG/start/data/executable
cp $EXECUTABLE $DAG/data/executable
echo " 11 1 " > $DAG/start/config/arguments
echo " 11 3 " > $DAG/config/arguments

# Work tasks (N+1)
for i in `seq 0 $N`; do
	mkdir $DAG/work_$i
	echo " 11 2 $i " > $DAG/work_$i/config/arguments
	cp $EXECUTABLE $DAG/work_$i/data/executable
	# Dependencies
	echo $ROOT > $DAG/work_$i/config/depends_on
	echo "work_$i" > $DAG/config/depends_on
done

# Launch
#echo start_and_wait > $DAG/control
