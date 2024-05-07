#include "./Server.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include "./ServerCluster.hpp"

const int BACKLOG = 100;

Server::Server()
{
}

Server::Server(unsigned int port, std::string host)
	: _host(inet_addr(host.data())), _port(port), _ip(host)
{
	Server::setup();
}

Server::~Server()
{
}

void Server::setup()
{
	// NEED TO ADJUST EXITS
	if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		exit(EXIT_FAILURE);

	int option_value = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = _host;
	_server_address.sin_port = htons(_port);

	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
		exit(EXIT_FAILURE);

	if (listen(_socket_fd, BACKLOG) == -1)
		exit(EXIT_FAILURE);

	std::cout << "Server started at http://" << _ip << ":" << _port << std::endl;
}

int Server::getSocketFd()
{
	return this->_socket_fd;
}
