# WEBSERV
 by ccattano bmacmaho jalbers

## Current open fronts:

* Carlo :
- [ ] Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- [ ] Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes is wrong, don't give any related points.
        - create execve error hadling
- [ ] Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
        -track fds flag on valgrind
- [X] POST  request is hanging on a cgi to folder or non existent
- [X] track all read/recv/write/send returns for errors
- [X] Upload some file to the server and get it back.




- [j] Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/)

- [X] Setup routes in a server to different directories.
- [X] Setup a default file to search for if you ask for a directory
- [X] Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).
- [j] redirected URL.
- [j] The CGI should be run in the correct directory for relative path file access.

## General:
- [b] File Upload
    - [b] Bug with uploaded/downloaded images
- [b] Timeout for requests/connections/cgiscipts - timer
- [b] Check if there is no hanging connection.


