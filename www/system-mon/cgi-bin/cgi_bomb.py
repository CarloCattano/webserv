#!/usr/bin/env python3

# produce a very large string of HTML
# of at last x2 4090 bytes

import time

def generate_html():
    html_content = f"""
<!doctype html>
<html>
<head>
    <title>Huge one</title>
    <link rel='stylesheet' href='../styles.css'>
</head>
<body>
"""

    for i in range(4090 * 2000):
        html_content += f"<p>{i}</p>\n"
        # generate large random string
        html_content += f"<p>{'a' * 40}</p>\n"



    html_content += f"""
</body>
</html>
"""

    return html_content

response_html = generate_html()
print(response_html)
