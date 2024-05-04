# To-Do:

### Allow Post and Get
Currently implemented a basic version where you can call "deny" and "allow" followed by methods and "all". Methods take precedence over the "all" keyword.
    - Need to find out how far this has to go... Do I need to implement "limit_except"?

### Execute CGI based on certain file extension (for example .php).
gpt:
To configure Nginix to run PHP scripts, you would have to use fastcgi_pass directive:
Example:
location ~ \.php$ {
    include /etc/nginx/fastcgi_params;
    fastcgi_pass 127.0.0.1:9000;
    fastcgi_index index.php;
    fastcgi_param SCRIPT_FILENAME /scripts$fastcgi_script_name;
}

 - Do I need to implement this extension matching? Isnt that?

### 8. Make the route able to accept uploaded files and configure where they should be saved.

For file uploads, you'll have to do some specific configuration and use a server side script to handle the saving of files. Below is an example of the kind of configuration needed:
Example:
location /upload {
    client_max_body_size 10m;
    fastcgi_pass   localhost:9000;
    fastcgi_param SCRIPT_FILENAME /scripts$fastcgi_script_name;
    include        fastcgi_params;
}

### 9. Your server should work with one CGI (PHP-CGI, Python, etc.)
gpt
This might be configuration that is outside of the Nginx configuration file itself. The important part is to set up Nginx to pass the correct information to FastCGI using the fastcgi_param directive.
For example:
location ~ \.php$ {
    include /etc/nginx/fastcgi_params;
    fastcgi_pass 127.0.0.1:9000;
    fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    fastcgi_param PATH_INFO $fastcgi_path_info;
    fastcgi_param PATH_TRANSLATED $document_root$fastcgi_path_info;
    fastcgi_split_path_info ^(.+\.php)(.*)$;
}
This example shows how to pass a .php file to a PHP-CGI handler. The fastcgi_param directives pass additional information to the PHP interpreter.