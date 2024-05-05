#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include "./Server.hpp"
#include "./ServerCluster.hpp"

Server::Server() {
}

Server::Server(unsigned int port, in_addr_t host) : _host(host), _port(port)
{
	Server::setup();
}

Server::~Server()
{
	close(_socket_fd);
}

void Server::setup()
{
	//NEED TO ADJUST EXITS
	if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0) )  == -1 )
        exit(EXIT_FAILURE);

    int option_value = 1;
    setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

	
    memset(&_server_address, 0, sizeof(_server_address));
	
    _server_address.sin_family = AF_INET;
    _server_address.sin_addr.s_addr = _host;
    _server_address.sin_port = htons(_port);


    if (bind(_socket_fd, (struct sockaddr *) &_server_address, sizeof(_server_address)) == -1)
        exit(EXIT_FAILURE);
}

int	Server::getSocketFd() {
	return this->_socket_fd;
}
