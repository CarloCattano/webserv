#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../Utils/FileUpload.hpp"
#include "Config.hpp"

class Server {
private:
	std::string			_host_name;
	int 				_port;
	struct sockaddr_in 	_server_address;
	int 				_socket_fd;

	void handle_file_request(int client_fd, const std::string &file_path);

	void handle_request(int fd);
	void handle_write(int fd);

	void handle_cgi_request(int client_fd, const std::string &cgi_script_path);
	void handle_static_request(int client_fd,
							   const std::string &requested_file_path,
							   const char *buffer);
	void handle_delete(int client_fd, std::string full_path, std::string file_path);

	static void stop(int signal);

	FileUploader uploader;

public:
	Server(const Virtual_Server_Config& server_config);
	~Server();

	void start();
	void start_listen();
	void await_connections();

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

	class InvalidPortException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return ("Invalid port");
		}
	};
};
#endif
