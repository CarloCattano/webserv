# WEBSERV

by ccattano bmacmaho jalbers

## Current open fronts:

./webserv conf/server.conf

* Carlo :

* Joseph:

* Barra


## hostnames check 
```shell
curl --resolve my42webserver.com:4222:127.0.0.1 http://my42webserver.com:4222/index.html
```

# bugs
- excess found with some requests with ``curl localhost:4222/index.html -v``
- run siege ( see docs/TESTING.md ) , sometimes there is one client left that hangs
    and the server CPU stays HIGH forever 
