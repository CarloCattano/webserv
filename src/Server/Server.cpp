#include "Server.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include "../Cgi/Cgi.hpp"
#include "../Utils/utils.hpp"

const int MAX_EVENTS = 100;
const int BACKLOG = 20;
const int BUFFER_SIZE = 1024;

std::string CGI_BIN = get_current_dir() + "/website/cgi-bin/" + "hello.py"; // TODO load from config

std::size_t extract_content_length(const char *request)
{
	const char *content_length_header = strstr(request, "Content-Length:");
	if (content_length_header != NULL) {
		// Skip the "Content-Length:" prefix
		content_length_header += strlen("Content-Length:");
		// Convert the value to size_t
		return std::strtoul(content_length_header, NULL, 10);
	}
	// If Content-Length header is not found or invalid, return 0
	return 0;
}

std::string extract_filename_from_request(const char *request)
{
	const char *filename_field = strstr(request, "filename=");
	if (filename_field != NULL) {
		const char *filename_start = filename_field + strlen("filename=");
		const char *filename_end = strstr(filename_start, "\r\n");
		if (filename_end != NULL) {
			std::string filename(filename_start, filename_end - filename_start);
			return filename;
		}
	}
	return "";
}

bool is_file_upload_request(const char *request)
{
	const char *content_type_header = strstr(request, "Content-Type:");
	if (content_type_header != NULL) {
		const char *multipart_form_data = strstr(content_type_header, "multipart/form-data");
		if (multipart_form_data != NULL) {
			return true;
		}
	}
	return false;
}

enum HttpMethod { GET, POST, DELETE, UNKNOWN };

HttpMethod get_http_method(const char *request)
{
	// Find the first space in the request line
	const char *first_space = strchr(request, ' ');
	if (first_space != NULL) {
		// Extract the HTTP method from the request line
		std::string method(request, first_space - request);
		if (method == "GET") {
			return GET;
		}
		else if (method == "POST") {
			return POST;
		}
		else if (method == "DELETE") {
			return DELETE;
		}
	}
	return UNKNOWN; // Unable to determine HTTP method
}

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port)
{
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = INADDR_ANY;
	_server_address.sin_port = htons(_port);
	Server::start();
}

void Server::stop(int signal)
{
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

/* takes care of the signal when a child process is terminated
	and the parent process is not waiting for it
	so it doesn't become a zombie process */

void handleSigchild(int sig)
{
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
}

void Server::start_listen()
{
	// TODO parse max port in config
	const int MAX_PORT = 65535;
	if (_port < 0 || _port > MAX_PORT)
		throw InvalidPortException();

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw SocketErrorException();

	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
		throw BindErrorException();

	if (listen(_socket_fd, BACKLOG) == -1)
		throw ListenErrorException();

	std::cout << "Server started at http://" << _ip_address << ":" << _port << std::endl;
}

void Server::await_connections()
{
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _socket_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	while (1) {
		struct epoll_event events[MAX_EVENTS];

		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		if (num_events == -1) {
			continue;
		}

		for (int i = 0; i < num_events; ++i) {
			if (events[i].data.fd == _socket_fd) {
				int client_fd = accept(_socket_fd, NULL, NULL);

				if (client_fd == -1) {
					perror("accept");
					continue;
				}

				ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
				ev.data.fd = client_fd;

				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
					perror("epoll_ctl");
					exit(EXIT_FAILURE);
				}
			}
			else {
				// message from existing client
				int client_fd = events[i].data.fd;
				if (client_fd == -1) {
					perror("events[i].data.fd");
					continue;
				}

				if (events[i].events & EPOLLIN) {
					handle_request(client_fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
				else if (events[i].events & EPOLLOUT) {
					handle_write(client_fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
				else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
			}
		}
	}
}

void Server::start()
{
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);

	start_listen();
	await_connections();
}

void Server::handle_write(int client_fd)
{
	char buffer[BUFFER_SIZE];
	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client_fd, buffer, BUFFER_SIZE, 0);

	if (size == -1) {
		perror("recv");
		return;
	}

	std::string request(buffer);

	if (is_file_upload_request(buffer)) {
		FileUploader uploader;
		std::size_t content_length = extract_content_length(buffer);
		std::cout << "Content length: " << content_length << std::endl;
		std::string filename = extract_filename_from_request(buffer);

		// we must extract the first part of the request body in between the boundary
		// to pass it to the file upload which will further extract the file content
		// but the first part of the request body is not in the buffer if we dont read it

		std::string content = extract_content_body(request.c_str());

		std::cout << "Content:\n" << content << std::endl;
		std::cout << "---------END OF CONTENT---------" << std::endl;

		uploader.handle_file_upload(client_fd, filename, content_length, content.c_str());
	}
	close(client_fd);
}

void Server::handle_request(int client_fd)
{
	char buffer[BUFFER_SIZE];
	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return;
	}

	if (size == -1) {
		perror("recv");
		return;
	}

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);
	std::string content_type = getContentType(requested_file_path);

	/* std::cout << "Raw Request: " << std::endl; */
	/* std::cout << buffer << std::endl; */

	if (requested_file_path.find(".py") != std::string::npos) {
		int forked = fork();

		if (forked == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		if (forked == 0) {
			Cgi cgi;
			std::string cgi_response = cgi.run(CGI_BIN);

			std::cout << content_type << "\n CONTENT TYPE -------\n " << std::endl;

			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
				"\r\nContent-Length: " + intToString(cgi_response.length()) + "\r\n\r\n" +
				cgi_response.c_str();

			send(client_fd, response.c_str(), response.size(), 0);
			close(client_fd);
			exit(0);
		}
		else {
			close(client_fd);
			return;
		}
	}
	else { // Handle static file request
		   // TODO filter http type
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
			"\r\nContent-Length: " + intToString(file_content.length()) + "\r\n\r\n" + file_content;

		send(client_fd, response.c_str(), response.size(), 0);
		close(client_fd);
	}
}

Server::~Server()
{
	close(_socket_fd);
}
