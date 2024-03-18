# python cgi test script with loop

import time
import os

# for loop that counts to 10

for i in range(10):
    print(f"<!DOCTYPE html>")
    print(f"<html>")
    print(f"<head>")
    print(f"<title>Counting to 10</title>")
    print(f"</head>")
    print(f"<body>")
    print(f"<h1>Counting to 10</h1>")
    print(f"<hr>")
    print(f"<p>{i}</p>")
    print(f"<hr>")
    print(f"</body>")
    print(f"</html>")
    time.sleep(0.2)

# end of script
