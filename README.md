# WEBSERV

by ccattano bmacmaho jalbers

## Current open fronts:

./webserv conf/server.conf

* Carlo :
	Setup the server_names or not.

* Joseph:

- [X] Setup routes in a server to different directories.
- [X] Setup a default file to search for if you ask for a directory
- [X] Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).
- [j] The CGI should be run in the correct directory for relative path file access.

* Barra


## hostnames check 
```shell
curl --resolve my42webserver.com:4222:127.0.0.1 http://my42webserver.com:4222/index.html
```

# bugs
- excess found with some requests with ``curl localhost:4222/index.html -v``
