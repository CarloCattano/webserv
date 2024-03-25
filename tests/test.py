import requests
import concurrent.futures
import time

# Function to send a single curl request
def send_curl_request(url):
    start_time = time.time()
    response = requests.post(url)
    end_time = time.time()
    response_time = end_time - start_time
    print(f"Response at {time.strftime('%H:%M:%S')} took {response_time:.2f} seconds")

# Function to send multiple concurrent requests
def send_concurrent_requests(url, num_requests):
    with concurrent.futures.ThreadPoolExecutor(max_workers=num_requests) as executor:
        executor.map(send_curl_request, [url] * num_requests)

# Main function
def main():
    url = "http://localhost:4222/cgi-bin/test.py"  # Adjust URL as needed
    num_requests = 10  # Adjust the number of concurrent requests
    send_concurrent_requests(url, num_requests)

if __name__ == "__main__":
    main()

