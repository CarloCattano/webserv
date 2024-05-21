#include "./Server.hpp"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/stat.h>

const int BACKLOG = 100;

Server::Server()
	: _port(0), _default_server(false), _client_max_body_size("") {
	_server_names = std::vector<std::string>();
	_error_pages = std::vector<std::string>();
	_routes = std::vector<Route>();

	Server::setup();
}

Server::Server(unsigned int port, std::string server_name)
	: _port(port)
	// : _host(inet_addr(host.data())), _port(port), _ip(host)
{
	std::vector<std::string>	server_names;

	server_names.push_back(server_name);
	_server_names = server_names;
	Server::setup();
}

Server::~Server()
{
}

void Server::setup()
{
	// NEED TO ADJUST EXITS
	if ((_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("socket");
	int option_value = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = inet_addr(_server_names[0].data());
	// _server_address.sin_addr.s_addr = _host;
	_server_address.sin_port = htons(_port);

	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
		perror("bind");
	if (listen(_socket_fd, BACKLOG) == -1)
		perror("listen");

	std::cout << "Server started at http://" << _server_names[0] << ":" << _port << std::endl;
}

int Server::getSocketFd()
{
	return this->_socket_fd;
}
