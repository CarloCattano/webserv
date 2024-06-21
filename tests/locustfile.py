from locust import HttpUser, task

#  locust -H http://127.0.0.1:4222

class HelloWorldUser(HttpUser):
    @task
    def hello_world(self):
        self.client.get("/index.html")
        self.client.get("http://127.0.0.2:4222/index.html")
        self.client.get("http://127.0.0.3:4222/index.html")
