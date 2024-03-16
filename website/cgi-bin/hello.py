import time

now = time.localtime()
hour = now.tm_hour

# start the HTML output
# print("Content-type: text/html\n")
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
print("<hr>")
print("</body></html>")

