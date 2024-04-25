#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Utils/FileUpload.hpp"
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

	void handle_request(int fd);
	void handle_write(int fd);
	static void stop(int signal);

	FileUploader uploader;

  public:
	Server(std::string ip_address, int port);
	~Server();

	void start();
	void start_listen();
	void await_connections();

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

	class InvalidPortException : public std::exception {
	  public:
		virtual const char *what() const throw() { return ("Invalid port"); }
	};
};
#endif
