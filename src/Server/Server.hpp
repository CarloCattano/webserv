#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include "../Utils/FileUpload.hpp"

class Server {
	private:
		in_addr_t			_host;
		int					_port;
		std::string			_ip;

		int					_socket_fd;
		struct sockaddr_in	_server_address;

	public:
		Server(unsigned int port, std::string host);
		Server();
		~Server();

		void setup();
		
		int	getSocketFd();
		in_addr_t getHost() {
			return _host;
		};
};
