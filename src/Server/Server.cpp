#include "Server.hpp"
#include "Cgi.hpp"
#include "utils.hpp"
#include <signal.h>
#include <sys/wait.h>

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

std::string CGI_BIN = "test.py"; // TODO load from config

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port) {
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = INADDR_ANY;
	_server_address.sin_port = htons(_port);
	start();
}

void Server::stop(int signal) {
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

void Server::start() {
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw SocketErrorException();

	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
		throw BindErrorException();

	if (listen(_socket_fd, BACKLOG) == -1)
		throw ListenErrorException();

	struct pollfd fds[MAX_EVENTS];
	fds[0].fd = _socket_fd;
	fds[0].events = POLLIN;

	std::cout << "Server started on http://localhost:" << _port << std::endl;

	// handle ctrl+c

	signal(SIGINT, stop);

	while (true) {
		int activity = poll(fds, MAX_EVENTS, -1);
		if (activity == -1) {
			perror("poll");
			continue;
		}

		if (fds[0].revents & POLLIN) {
			int client_fd = accept(_socket_fd, NULL, NULL);
			if (client_fd == -1) {
				perror("accept");
				continue;
			}

			handle_request(client_fd);
			close(client_fd);
		}
	}
	close(_socket_fd);
}

void Server::handle_request(int client_fd) {
	char buffer[BUFFER_SIZE];
	int size = recv(client_fd, buffer, BUFFER_SIZE, 0);

	if (size == -1) {
		perror("recv");
		return;
	}

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);
	std::string content_type = getContentType(requested_file_path);

	if (requested_file_path.find(".py") != std::string::npos) {
		// Start CGI execution asynchronously
		pid_t pid = fork();
		if (pid == -1) {
			perror("fork");
			return;
		}

		if (pid == 0) {
			try {
				Cgi cgi;
				std::string cgi_response = cgi.run(CGI_BIN);

				std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
									   "\r\nContent-Length: " + intToString(cgi_response.length()) +
									   "\r\n\r\n" + cgi_response.c_str();
				send(client_fd, response.c_str(), response.size(), 0);
				exit(0);
			} catch (std::exception &e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
			close(client_fd);
			exit(0);
		} else { // Parent process
			// Non-blocking wait for the child process to complete
			int status;
			waitpid(pid, &status, WNOHANG);
		}

	} else if (file_content.empty()) {

		std::string errResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
		send(client_fd, errResponse.c_str(), errResponse.size(), 0);

	} else {
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
							   "\r\nContent-Length: " + intToString(file_content.length()) +
							   "\r\n\r\n" + file_content;

		send(client_fd, response.c_str(), response.size(), 0);
	}

	close(client_fd);
}

Server::~Server() { close(_socket_fd); }
