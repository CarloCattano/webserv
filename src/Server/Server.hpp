#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.hpp"

class Server {
private:
	std::string _ip_address;
	struct sockaddr_in _server_address;
	int _port;
	int _socket_fd;
	// int 				_client_socket_fd;

	void handle_request(int fd);

public:
	Server(std::string ip_address, int port);
	~Server();

	void start();

	class BindErrorException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return ("Bind error");
		}
	};

	class SocketErrorException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return ("Socket error");
		}
	};

	class ListenErrorException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return ("Listen error");
		}
	};
};

#endif
