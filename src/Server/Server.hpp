#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include "../Utils/FileUpload.hpp"

class Server {
	private:
		in_addr_t			_host;
		int					_port;
		int					_socket_fd;
		struct sockaddr_in	_server_address;

	public:
		Server(unsigned int port, in_addr_t host);
		Server();
		~Server();

		void setup();
		
		int	getSocketFd();
};
