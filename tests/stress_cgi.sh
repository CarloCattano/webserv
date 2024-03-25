#!/bin/bash

# log time difference between requests to a cgi script
T_DIFF=0

# loop $1 times and performa a curl post 

# usage 
if [ $# -ne 1 ]
then
   echo "Usage: $0 <number of times to loop>"
   exit 1
fi

for i in $(seq 1 $1)
do
   T_START=$(date +%s)
   curl -X POST  http://localhost:4222/cgi-bin/test.py &> /dev/null 
   T_END=$(date +%s)
   T_DIFF=$(($T_END - $T_START))
   echo "Response at $(date +%H:%M:%S) took $T_DIFF seconds"
done


