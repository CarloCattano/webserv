# WEBSERV
 by ccattano bmacmaho jalbers

## Current open fronts:
- [ ] Parsing implementation (See TODO tags)
- [ ] Request handling

- [ ] Response handling
- [ ] Cluster management

- [ ] File Upload 
- [ ] Directory listing
- [ ] take cgi script name and execute it if it exists in the associated directory ( currently hardcoded to either hello.py or test.py)
- [ ] finishing DELETE METHOD CHECKS
- [ ] test infinite loop and revisit timeout method again 
        - timeout works but it uses alarm, which is a function from unistd.h and not allowed
## weird bugs:
    - [ ] `curl -X POST localhost:4222/hello.py` fails sometimes, with perror recv: bad file descriptor
    

## Many hardcoded values such as:
- [ ] Port number
- [ ] Server root
- [ ] Server name
- [ ] autoindex
- [ ] cgi paths
- [ ] server website root
- [ ] server website index

