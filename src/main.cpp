#include <iostream>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Cgi.hpp"
#include "utils.hpp"

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

int PORT = 8080; // TODO needs to be set by config file

void populateContentTypes()
{
	content_types[".html"] = "text/html";
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	content_types[".py"] = "text/plain";
	// Add more file extensions and corresponding content types as needed
}

void handle_request(int client_fd)
{
	char buffer[BUFFER_SIZE];
	int size = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (size == -1) {
		perror("recv");
		return;
	}

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);

	// cgi script
	if (requested_file_path.find(".py") != std::string::npos) {
		Cgi cgi;
		std::string response = cgi.run();
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else if (file_content.empty()) {
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

		if (content_type != "image/jpeg" && content_type != "image/png") {
			std::cout << "Response:\n-----\n" << response << std::endl;
		}
		send(client_fd, response.c_str(), response.size(), 0);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}

	// TODO Parse configuration here

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("socket");
		return 1;
	}

	populateContentTypes();

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		perror("bind");
		return 1;
	}

	if (listen(server_fd, BACKLOG) == -1) {
		perror("listen");
		return 1;
	}

	struct pollfd fds[MAX_EVENTS];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	std::cout << "Server listening on http://localhost:" << PORT << std::endl;

	while (true) {
		int activity = poll(fds, MAX_EVENTS, -1);
		if (activity == -1) {
			perror("poll");
			continue;
		}

		if (fds[0].revents & POLLIN) {
			int client_fd = accept(server_fd, NULL, NULL);
			if (client_fd == -1) {
				perror("accept");
				continue;
			}

			handle_request(client_fd);
			close(client_fd);
		}
	}

	close(server_fd);

	return 0;
}
