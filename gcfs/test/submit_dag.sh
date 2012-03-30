#!/bin/sh

# Set GCFS location
GCFS="/mnt/gcfs"
DAG=$GCFS/dag_root

# Create tasks
mkdir $DAG
mkdir $DAG/task1

# Copy executables and set arguments
cp /usr/bin/touch $DAG/data/executable
cp /bin/ls $DAG/task1/data/executable

echo " test1" > $DAG/config/arguments
echo " -l " > $DAG/task1/config/arguments

# Set dependency -> task "task1" denepds on "dag_root"
echo "dag_root" > $DAG/task1/config/depends_on

# Launch
echo start_and_wait > $DAG/control
