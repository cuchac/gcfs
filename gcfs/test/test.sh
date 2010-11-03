#!/bin/sh

echo "Creating task 'test'"
mkdir ./gcfs/test

echo "Using SAGA ..."
echo saga > ./gcfs/test/config/service

echo -n "Task status: "
cat ./gcfs/test/status

echo "Copying /bin/cp as binary executable"
cp /bin/cp ./gcfs/test/data/executable

echo "Setting argument to '-v fileIn1 fileOut' - copy verbosely file"
echo "-v fileIn1 fileOut" > ./gcfs/test/config/arguments

echo "Creating data files 'fileIn1' and 'fileIn2' with its name as content"
echo fileIn1 > ./gcfs/test/data/fileIn1
echo fileIn2 > ./gcfs/test/data/fileIn2

echo -n "Content of data folder:"
ls ./gcfs/test/data/

echo "Starting ..."
echo start > ./gcfs/test/control

echo -n "Task status: "
cat ./gcfs/test/status

echo "Waiting ...."
echo wait > ./gcfs/test/control

echo -n "Task status: "
cat ./gcfs/test/status

echo "Finished waiting!"

echo -n "Content of result folder: "
ls ./gcfs/test/result

#echo "Deleting task 'test'"
#rmdir ./gcfs/test
