#include "./Server.hpp"

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

int PORT = 8080; // TODO needs to be set by config file

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port) {
	
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = INADDR_ANY;
	_server_address.sin_port = htons(_port);
	start();
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

	std::cout << "Server listening on http://localhost:" << PORT << std::endl;

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
}

void Server::handle_request(int client_fd)
{
	char buffer[BUFFER_SIZE];
	int size = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (size == -1) {
		perror("recv");
		return;
	}

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);

	if (file_content.empty()) {
		// File not found or error reading file
		std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else {
		// Determine content type based on file extension
		std::string content_type = getContentType(requested_file_path);

		// Construct HTTP response
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
			"\r\nContent-Length: " + intToString(file_content.length()) + "\r\n\r\n" + file_content;

		send(client_fd, response.c_str(), response.size(), 0);
	}
}

Server::~Server() {
	close(_socket_fd);
}