#!/bin/bash

# log time difference between requests to a CGI script
T_DIFF=0

# Function to send curl request
send_curl_request() {
    T_START=$(date +%s)
    curl -X POST http://localhost:4222/cgi-bin/test.py &> /dev/null 
    T_END=$(date +%s)
    T_DIFF=$(($T_END - $T_START))
    echo "Response at $(date +%H:%M:%S) took $T_DIFF seconds"
}

# Usage
if [ $# -ne 1 ]; then
    echo "Usage: $0 <number of requests>"
    exit 1
fi

# Number of parallel requests
NUM_REQUESTS=$1

# Launch requests in parallel
for i in $(seq 1 $NUM_REQUESTS); do
    send_curl_request &
done

# Wait for all background processes to finish
wait

