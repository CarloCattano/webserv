#!/usr/bin/env python

import sys
import time
import threading
import requests

def test():
    try:
        response = requests.get('http://localhost:4222')
        #time of response

        print(response.status_code, response.elapsed.total_seconds())
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
        t.start()

    for t in threads:
        t.join()

if __name__ == '__main__':
    main()
