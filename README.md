# WEBSERV
 by ccattano bmacmaho jalbers

## Current open fronts:
- [ ] Parsing implementation (See TODO tags)

- [ ] File Upload 
- [ ] take cgi script name and execute it if it exists in the associated directory ( currently hardcoded to either hello.py or test.py)
- [ ] test infinite loop and revisit timeout method again 
        - timeout works but it uses alarm, which is a function from unistd.h and not allowed

### Many hardcoded values such as:
- [ ] Server name
- [ ] server website index


### Double check:
    - [ ] wrong path or dir requested
