#include <cstdlib>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

std::string CGI_BIN = "/home/carlo/42/webserv/website/cgi-bin/test.py"; // Path to your CGI script

void handle_request(int client_fd)
{
	std::cout << "Handling request" << std::endl;
	std::cout << "Client fd: " << client_fd << std::endl;

	char buffer[1024];
	int size = recv(client_fd, buffer, sizeof(buffer), 0);

	if (size == -1) {
		perror("recv");
		return;
	}

	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		return;
	}

	if (pid == 0) {
		close(STDOUT_FILENO);			// Close stdout to redirect it to the socket
		dup2(client_fd, STDOUT_FILENO); // Redirect stdout to the client socket
		close(client_fd);				// Close the original client socket

		char *argv[] = { (char *)"/usr/bin/python3", (char *)CGI_BIN.c_str(), NULL };

		if (execve("/usr/bin/python3", argv, NULL) == -1) {
			perror("execve");
			exit(EXIT_FAILURE);
		}
	}
	else {
		int status;
		waitpid(pid, &status, WNOHANG); // Wait for the child process to finish
										// Send a basic HTTP response header
		std::string response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
		send(client_fd, response_header.c_str(), response_header.size(), 0);
		close(client_fd);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}

	int server_fd, client_fd;
	struct sockaddr_in server_address, client_address;
	socklen_t client_address_size = sizeof(client_address);

	// Create a TCP socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("socket");
		return 1;
	}

	// Set socket options to allow address reuse
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
		perror("setsockopt");
		return 1;
	}

	// Bind the socket to the server address and port
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8080); // Change this to your desired port
	if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
		perror("bind");
		return 1;
	}

	// Listen for incoming connections
	if (listen(server_fd, 5) == -1) {
		perror("listen");
		return 1;
	}

	while (true) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
		if (client_fd == -1) {
			perror("accept");
			continue;
		}
		std::cout << "Connection accepted" << std::endl;

		handle_request(client_fd);
	}

	close(server_fd);

	return 0;
}
