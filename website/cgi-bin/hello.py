import time

now = time.localtime()
hour = now.tm_hour

import os

with open('/proc/loadavg', 'r') as f:
    load_avg_data = f.readline().split()
    load_avg_1_min = float(load_avg_data[0])
    num_cores = os.cpu_count()  # Get the number of CPU cores
    cpu_utilization_percent = (load_avg_1_min / num_cores) * 100
    load_avg = cpu_utilization_percent 
    load_avg = round(load_avg, 2)

print("<html><head><title>Greetings</title></head><body>")
print("<h1>Greetings</h1>")
print("<hr>")
print("<p>")
print("Good ")
if hour < 12:
    print("morning")
elif hour < 18:
    print("afternoon")
else:
    print("evening")
print("</p>")
print("<br>")
print("<h2>System load average : ")
print(load_avg)
print(" %")
print("</h2>")
print("<hr></body></html>")

