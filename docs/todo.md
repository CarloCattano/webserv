
MIME types for files upload
 - https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types

RFC only check the MUST 

handle bind error with ports in use or not permitted 

example of bind error
```
listen 422

terminate called after throwing an instance of 'Server::BindErrorException'
  what():  Bind error
make: *** [Makefile:19: run] Aborted (core dumped)
```

- Load the cgi-bin python script and cgi-bin allowed directory from the 
  nginx style configuration file

### Requests and Responses

 * Abstract response/request headers, status code, and body to a class

 * Filter requests by type and path

 * Check for allowed paths

 * Check for allowed methods

 * Check for allowed headers

* fix hardcoded path extraction from the request



### Updated TO-DO

 * Localhost server name not working

 * Handle multiple servernames (its an array)

 * Update all TODO comments left in code

 * Use autoindex setting on server level

 * 

