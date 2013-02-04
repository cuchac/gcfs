#!/bin/sh

CGROUP="/stroll"
LOG=${1:-"measure.log"}

#######################################################
################## Functions ##########################
#######################################################

# Returns string representation of current time in nanoseconds
function getCurrentTime {
	echo $(date "+%s%N")
}

# Returns string representation of CPU time consumed by $1 cgroup
function getCgroupCPUTime {
	echo $(cgget -n -v -r cpuacct.usage $1 | tail -n 1)
}

# Returns string representation of memory consumed by $1 cgroup
function getCgroupMemory {
	echo $(ps -eo rss,cgroup | grep "cpuacct:"$1 | awk '{s+=$1}END{print s}')
}

# Returns string representation of floating point $1/$2
function divide {
	local precission
	precission=${3:-4}
	echo "scale=$precission;$1/$2" | bc
}

#######################################################
#################### Main #############################
#######################################################

echo>$LOG

START_TIME=$(getCurrentTime)
START_CPU_TIME=$(getCgroupCPUTime $CGROUP)
START_MEMORY=$(getCgroupMemory $CGROUP)

while true
do
	TIME=$(getCurrentTime)
	CPU_TIME=$(getCgroupCPUTime $CGROUP)
	MEMORY=$(getCgroupMemory $CGROUP)

	let TIME=$TIME-$START_TIME
	let CPU_TIME_DIFF=$CPU_TIME-$START_CPU_TIME
	let MEMORY=$MEMORY-$START_MEMORY
	START_CPU_TIME=$CPU_TIME

	echo "$(divide $TIME 1000000000) 		$(divide $CPU_TIME_DIFF 1000000) 		$MEMORY" >> $LOG
	echo "$(divide $TIME 1000000000) 		$(divide $CPU_TIME_DIFF 1000000) 		$MEMORY		"$(ps -eo rss,cgroup | grep "cpuacct:"$CGROUP | wc -l)

	sleep 0.1
done;



