#!/usr/bin/env python3
import requests
import concurrent.futures
import time
import sys


# keep track of each request number 
request_number = 0

def send_curl_request(url):
    start_time = time.time()
    response = requests.post(url)
    end_time = time.time()
    response_time = end_time - start_time
    global request_number
    request_number += 1
    print(f"Response took {response_time:.2f} seconds" + f" STATUS:\t {response.status_code}" + f" {request_number}")

def send_concurrent_requests(url, num_requests):
    with concurrent.futures.ThreadPoolExecutor(max_workers=num_requests) as executor:
        executor.map(send_curl_request, [url] * num_requests)

# Main function
def main():
    num_requests = int(sys.argv[1])
    url = "http://localhost:4222/cgi-bin/" + sys.argv[2]  # Adjust URL as needed
    send_concurrent_requests(url, num_requests)

if __name__ == "__main__":
    main()

