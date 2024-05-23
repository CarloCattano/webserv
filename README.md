# WEBSERV
 by ccattano bmacmaho jalbers

## Current open fronts:
- [ ] Parsing implementation (See TODO tags)
- [ ] Request handling

- [ ] Response handling

- [ ] File Upload 
- [ ] take cgi script name and execute it if it exists in the associated directory ( currently hardcoded to either hello.py or test.py)
- [ ] test infinite loop and revisit timeout method again 
        - timeout works but it uses alarm, which is a function from unistd.h and not allowed

### Many hardcoded values such as:
- [ ] Port number
- [ ] Server root
- [ ] Server name
- [ ] autoindex
- [ ] cgi paths
- [ ] server website root
- [ ] server website index
