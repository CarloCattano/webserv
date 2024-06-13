#!/usr/bin/env python3

from bs4 import BeautifulSoup as bs
import requests, time

class WeatherScrap:
    def __init__(self):
        self.USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36"
        self.LANGUAGE = "en-US,en;q=0.5"
        self.next_days = []

    def get_weather_icon_filename(self,weather_condition):
        weather_icons = {
            "Clear": "sunny.png",
            "Partly cloudy": "cloudy.png",
            "Cloudy": "cloudy.png",
            "Overcast": "cloudy.png",
            "Fog": "cloudy.png",
            "Light rain": "rainy.png",
            "Moderate rain": "rainy.png",
            "Heavy rain": "rainy.png",
            "Thunderstorm": "rainy.png",
            "Light snow": "rainy.png",
            "Moderate snow": "rainy.png",
            "Heavy snow": "rainy.png",
            "Hail": "rainy.png",
            "Sleet": "rainy.png",
            "Freezing rain": "rainy.png",
            "Showers": "rainy.png",
        }
        return weather_icons.get(weather_condition, "current.png")

    def get_weather_data(self):
        start_time = time.time()
        session = requests.Session()
        session.headers['User-Agent'] = self.USER_AGENT
        session.headers['Accept-Language'] = self.LANGUAGE
        session.headers['Content-Language'] = self.LANGUAGE
        html = session.get("https://www.google.com/search?lr=lang_en&ie=UTF-8&q=weather&hl=en",timeout=1)
        soup = bs(html.text, "html.parser")

        result = {}
        result['region'] = soup.find("div", attrs={"id": "wob_loc"}).text
        result['temp_now'] = soup.find("span", attrs={"id": "wob_tm"}).text
        result['dayhour'] = soup.find("div", attrs={"id": "wob_dts"}).text
        result['weather_now'] = soup.find("span", attrs={"id": "wob_dc"}).text
        result['precipitation'] = soup.find("span", attrs={"id": "wob_pp"}).text
        result['humidity'] = soup.find("span", attrs={"id": "wob_hm"}).text
        result['wind'] = soup.find("span", attrs={"id": "wob_ws"}).text
        next_days = []
        days = soup.find("div", attrs={"id": "wob_dp"})


        for day in days.findAll("div", attrs={"class": "wob_df"}):
            day_name = day.findAll("div")[0].attrs['aria-label']
            weather = day.findAll("img")[0].attrs['alt']
            temp = day.findAll("span", {"class": "wob_t"})
            max_temp = temp[0].text
            min_temp = temp[2].text
            next_days.append({"name": day_name, "weather": weather, "max_temp": max_temp, "min_temp": min_temp})

        self.next_days = next_days
        
        end_time = time.time()
        result['time'] = end_time - start_time
        return result

    def get_next_days(self):
        return self.next_days

# generate html site from weather data

    def generate_html(self, weather_data, next_days):
        html = """
        <!DOCTYPE html>
        <html>
        <head>
        <title>Weather</title>
        <link rel="stylesheet" type="text/css" href="../weather.css">
        <link rel="icon" type="image/png" href="../assets/favicon.png">
        </head>
        <body>
        <div class="weather-container">
            <h1>Weather in Berlin Now</h1>
            <div class="current-weather">
                <img src="../assets/current.png" alt="Current Weather" class="weather-icon">
                <div class="weather-info">
                    <p><strong>Temperature now:</strong> {temp_now}&deg;C </p>
                    <p><strong>Day and hour:</strong> {dayhour}</p>
                    <p><strong>Weather now:</strong> {weather_now}</p>
                    <p><strong>Precipitation:</strong> {precipitation}</p>
                    <p><strong>Humidity:</strong> {humidity}</p>
                    <p><strong>Wind:</strong> {wind}</p>
                </div>
            </div>
            <h2>Next Days</h2>
            <ul class="forecast-list">
        """.format(region=weather_data['region'], temp_now=weather_data['temp_now'], dayhour=weather_data['dayhour'],
                   weather_now=weather_data['weather_now'], precipitation=weather_data['precipitation'], humidity=weather_data['humidity'], wind=weather_data['wind'])
        
        for day in next_days:
            weather_icon = self.get_weather_icon_filename(day['weather'])
            html += """
            <li class="forecast-item">
                <img src="../assets/{icon}" alt="{weather}" class="weather-icon">
                <div class="weather-info">
                    <p><strong>{name}:</strong> {weather}, max temp: {max_temp}&deg;C, min temp: {min_temp}&deg;C</p>
                </div>
            </li>
            """.format(name=day['name'], weather=day['weather'], max_temp=day['max_temp'], min_temp=day['min_temp'], icon=weather_icon)
    
        html += """
            </ul>
        </div>
        </body>
        </html>
        """
        return html

if __name__ == "__main__":
    weather = WeatherScrap()
    weather_data = weather.get_weather_data()
    next_days = weather.get_next_days()
    html_content = weather.generate_html(weather_data, next_days)
    print(html_content)

