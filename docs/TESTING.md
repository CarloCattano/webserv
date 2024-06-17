```shell
tests/test_cgi.py 80 sysmonitor.py
```

```shell
siege -b -t 4s -c 255 -f conf/siege.conf
```

```shell
curl -X DELETE http://127.0.0.2:4222/filename.extension
```

```shell
curl --resolve my42webserver.com:4222:127.0.0.1 http://my42webserver.com:4222/index.html
```

```shell
valgrind  --leak-check=full  --show-leak-kinds=all  --track-fds=yes ./webserv server.conf 
```

