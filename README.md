# WEBSERV
 by ccattano bmacmaho jalbers


rework

## Current open fronts:

- [c ] File Upload 
- [c ] track all read/recv/write/send returns
- [c ] Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- [c ] Upload some file to the server and get it back.
        - hangs on submission
- [c ] POST  request is hanging on a cgi to folder or non existent



- [j] Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/)
- [j] Setup routes in a server to different directories.
- [j] Setup a default file to search for if you ask for a directory
- [j] Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).
- [j] The CGI should be run in the correct directory for relative path file access.

## General:
- [b] Timeout for requests/connections/cgiscipts - timer
- [b] redirected URL.
- [b] Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
        -track fds flag on valgrind
- [b] Check if there is no hanging connection.
- [B] Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes is wrong, don't give any related points.
        - create execve error hadling

