# WEBSERV
 by ccattano bmacmaho jalbers

## Current open fronts:

./webserv conf/server.conf

* Carlo :

- [X] Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
- [X] Fix the download of files in directory listing 
        - It doest work on file-upload server on /upload

- [X] Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes is wrong, don't give any related points.
        - wrong file 
- [X] Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- [X] Execve error handling
- [X] POST  request is hanging on a cgi to folder or non existent
- [X] track all read/recv/write/send returns for errors
- [X] Upload some file to the server and get it back.


* Joseph:

- [j] Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/)

- [X] Setup routes in a server to different directories.
- [X] Setup a default file to search for if you ask for a directory
- [X] Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).
- [j] redirected URL.
- [j] The CGI should be run in the correct directory for relative path file access.

* Barra
- [X] File Upload
    - [b] Bug with uploaded/downloaded images
- [b] Timeout for requests/connections/cgiscipts - timer
- [b] Check if there is no hanging connection.



## hostnames check 
```shell
curl --resolve my42webserver.com:4222:127.0.0.1 http://my42webserver.com:4222/index.html
```

# bugs
- excess found with some requests with ```curl localhost:4222/index.html -v```

