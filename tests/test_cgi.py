import requests
import concurrent.futures
import time
import sys

def send_curl_request(url):
    start_time = time.time()
    response = requests.post(url)
    print(response.text)
    end_time = time.time()
    response_time = end_time - start_time
    print(f"Response at {time.strftime('%H:%M:%S')} took {response_time:.2f} seconds")

def send_concurrent_requests(url, num_requests):
    with concurrent.futures.ThreadPoolExecutor(max_workers=num_requests) as executor:
        executor.map(send_curl_request, [url] * num_requests)

# Main function
def main():
    num_requests = int(sys.argv[1])
    url = "http://localhost:4222/cgi-bin/infinite_loop.py"  # Adjust URL as needed
    send_concurrent_requests(url, num_requests)

if __name__ == "__main__":
    main()

