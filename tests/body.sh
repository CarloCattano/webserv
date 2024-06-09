#!/bin/bash

url="http://localhost:4222/upload"
content_type="application/octet-stream"
file="testfile.dat"
initial_size=1
increment=1
max_size=101

# syntax check and usage message
if [[ "$#" -ne 0 ]]; then
    echo "Usage: $0"
    exit 1
fi

for ((size=initial_size; size<=max_size; size+=increment)); do
    echo "Testing with file size ${size} MB"
    dd if=/dev/zero of=$file bs=1M count=$size &> /dev/null
    response=$(curl -X POST $url \ -H "Content-Type: application/octet-stream" \
     -H "Content-Disposition: attachment; filename=\"largefile.dat\"" --data-binary @largefile.dat -v)
    echo "Response code: $response"
    if [[ "$response" != "200" ]]; then
        echo "Server failed to handle ${size} MB"
        break
    fi
done

rm -f $file
