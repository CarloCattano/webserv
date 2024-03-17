#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Server {
  private:
	std::string _ip_address;
	struct sockaddr_in _server_address;
	int _port;
	int _socket_fd;
	// int 				_client_socket_fd;

	void handle_request(int fd);
	static void stop(int signal);

  public:
	Server(std::string ip_address, int port);
	~Server();

	void start();

	class BindErrorException : public std::exception {
	  public:
		virtual const char *what() const throw() { return ("Bind error"); }
	};

	class SocketErrorException : public std::exception {
	  public:
		virtual const char *what() const throw() { return ("Socket error"); }
	};

	class ListenErrorException : public std::exception {
	  public:
		virtual const char *what() const throw() { return ("Listen error"); }
	};
};

#endif
