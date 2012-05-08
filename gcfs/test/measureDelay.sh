#!/bin/sh -x

# This script monitors execution of Condor task using GCFS
# Script monitors task submission memory usage of GCFS
# and measures time necessary to submit all tasks to Condor

###########################################################
################### DEFINES ###############################
###########################################################

GETPID_COMMAND="ps -o pid -C "
CONDOR_Q="condor_q"
GCFS_EXECUTABLE="gcfs"

###########################################################
################## FUNCTIONS ##############################
###########################################################

# Print help
function printHelp() {
	echo "Usage: $0 <task directory>"
	exit 1
}

# Get PID(s) of task(s) with executable name specified in $1
function getPID() {
	local pids="$($GETPID_COMMAND $1)"
	local lines=$(echo -e "$pids" | wc -l)
	[[ $lines < 2 ]] && echo "Cannot find PID of process $1" && return 1
	echo $(echo -e "$pids" | tail -n +2)
}

# Get Maximum Memory of task with PID specified in $1
function getMemMax() {
	set $(echo $1 | tr -d "")
	cat /proc/$1/status | grep -e '^VmHWM' | tr -s " " | cut -d " " -f 2
}

# Get Total Maximum Memory of tasks with PIDs specified in $1
function getTotalMemMax {
	local result=0
	for i in $@
	do
		let result=$result+$(getMemMax $i)
	done
   echo $result
}

# Wait until all of the $2 tasks of cluster task $1 is submitted (appears in condor_q)
function waitForSubmission() {
	local finished=0
	for i in seq 0 $2
	do
		finished=0
		while [ "$finished" != "1" ]
		do
			finished=$($CONDOR_Q | grep $1.$i | wc -l)
		done
	done
}

###########################################################
##################### MAIN ################################
###########################################################

[ $# != 2 ] && printHelp || exit

TASKDIR=$1

# Get number of processes
PROCESSES=$(cat "$TASKDIR/config/processes")

# Measure original mem size
PID=$(getPID $GCFS_EXECUTABLE)
MEM_ORIG=$(getMemMax $PID)
echo "PID of GCFS process: "$PID
echo "Original max memory used by GCFS process: $MEM_ORIG kB"

# Get start time
TIME_ORIG=$(date "+%s%N")

# Launch the submission script
echo "start" > $TASKDIR/control

#Get cluster ID of submitted task
CLUSTER_ID=$(cat $TASKDIR/status | tail -n 1)
[ -z $CLUSTER_ID] && echo "Cannot get Cluecter ID!!!" && exit 2

# Loop and wait for all tasks to be submitted
waitForSubmission $CLUSTER_ID $PROCESSES

# Get end time
TIME_FINAL=$(date "+%s%N")

# Count time difference
TIME_DIFF=0
let TIME_DIFF=($TIME_FINAL-$TIME_ORIG)/1000000

ech "Time of submission: $TIME_DIFF ms"

# Measure current mem size and calculate difference
MEM_FINAL=$(getMemMax $PID)
MEM_DIFF=0
let MEM_DIFF=$MEM_FINAL-$MEM_ORIG
echo "Final max memory used by GCFS processes: $MEM_FINAL kB, difference: $MEM_DIFF kB"

