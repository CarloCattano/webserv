# WEBSERV
 by ccattano bmacmaho jalbers
## Current open fronts:

* Carlo :

- [ ] Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
        -track fds flag on valgrind
        - check where to clear and modify destructors
- [ ] Fix the download of files in directory listing 
        - It doest work on file-upload server on /upload

- [X] Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes is wrong, don't give any related points.
        - wrong file 
- [X] Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit")
- [X] Execve error handling
- [X] POST  request is hanging on a cgi to folder or non existent
- [X] track all read/recv/write/send returns for errors
- [X] Upload some file to the server and get it back.

* Joseph:
- [ ] Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/)

- [ ] Setup routes in a server to different directories.
- [ ] Setup a default file to search for if you ask for a directory
- [ ] Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).
- [ ] redirected URL.
- [ ] The CGI should be run in the correct directory for relative path file access.

* Barra:
- [ ] File Upload 
    - [ ] Bug with uploaded/downloaded images
- [ ] Timeout for connections - timer
- [ ] Check if there is no hanging connection.

# bugs
- excess found with some requests with ```curl localhost:4222/index.html -v```
