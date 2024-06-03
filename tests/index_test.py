#!/usr/bin/env python

import sys
import threading
import requests
import time

def test():
    try:
        response = requests.put('http://'+ sys.argv[2], timeout=5)
        print(response.text)
        print(response.status_code, response.elapsed.total_seconds().__round__(2))
    except Exception as e:
        print(e)

def main():
    try:
        arg1 = int(sys.argv[1])
    except Exception as e:
        print(e)
        sys.exit(1)

    threads = []
    for i in range(arg1):
        t = threading.Thread(target=test)
        threads.append(t)
        # time.sleep(0.2)
        t.start()

    for t in threads:
        t.join()

if __name__ == '__main__':
    main()

