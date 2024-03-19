import time
import os

def get_load_average():
    with open('/proc/loadavg', 'r') as f:
        load_avg_data = f.readline().split()
        load_avg_1_min = float(load_avg_data[0])
        num_cores = os.cpu_count()  # Get the number of CPU cores
        cpu_utilization_percent = (load_avg_1_min / num_cores) * 100
        return round(cpu_utilization_percent, 2)

def get_greeting():
    now = time.localtime()
    hour = now.tm_hour
    if hour < 12:
        return "Good morning"
    elif hour < 18:
        return "Good afternoon"
    else:
        return "Good evening"

def generate_html():
    load_avg = get_load_average()
    greeting = get_greeting()

    html_content = f"""
    <html>
    <head>
        <title>Greetings</title>
        <link rel='stylesheet' href='../styles.css'>
    </head>
    <body>
        <h1>Greetings</h1>
        <hr>
        <p>{greeting}</p>
        <br>
        <h2>System load average : {load_avg} %</h2>
        <img src='../assets/cpu.gif' />
        <hr>
        <form>
            <input type='button' value='Back' onclick='goBack()'>
        </form>
        <script>
            function goBack() {{
                window.history.back();
            }}
        </script>
    </body>
    </html>
    """

    return html_content

response_html = generate_html()

print(response_html)
