#include "Server.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../Cgi/Cgi.hpp"
#include "../Utils/utils.hpp"

const int MAX_EVENTS = 100;
const int BACKLOG = 20;
const int BUFFER_SIZE = 1024;
const bool autoindex = true; // TODO load from config

std::string CGI_BIN = get_current_dir() + "/website/cgi-bin/" + "hello.py"; // TODO load from config

// Server::Server(std::string host_name, int port) : _host_name(host_name), _port(port)
Server::Server(const Virtual_Server_Config& server_config)
{
	_host_name = server_config.server_names[0];
	_port = server_config.port;
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = INADDR_ANY;
	_server_address.sin_port = htons(_port);
	Server::start();
}

Server::~Server()
{
	close(_socket_fd);
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

	std::cout << "Server started at http://" << _host_name << ":" << _port << std::endl;
}

int	Server::create_epoll_instance()
{
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	return (epoll_fd);
}

struct epoll_event Server::create_epoll_event_structure()
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _socket_fd;
	return (ev);
}

int add_fd_to_epoll_instance(int epoll_fd, int event, int new_fd, epoll_event* ev) {
	if (epoll_ctl(epoll_fd, event, new_fd, ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
	return (0);
}

void Server::await_connections()
{
	int 				epoll_fd = create_epoll_instance();
	struct epoll_event 	ev = create_epoll_event_structure();

	add_fd_to_epoll_instance(epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev);

	while (1) { // TODO add a flag to run the server
		struct epoll_event events[MAX_EVENTS];

		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (num_events == -1)
			continue;

		for (int i = 0; i < num_events; ++i) {
			if (events[i].data.fd == _socket_fd) {
				int client_fd = accept(_socket_fd, NULL, NULL);

				if (client_fd == -1) {
					perror("accept");
					continue;
				}

				ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
				ev.data.fd = client_fd;

				add_fd_to_epoll_instance(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
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

	HttpMethod reqType = get_http_method(buffer);

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);
	std::string content_type = getContentType(requested_file_path);
	std::string full_path = "website/";

	if (requested_file_path.find(".py") != std::string::npos && reqType == POST) {
		handle_cgi_request(client_fd, CGI_BIN); // TODO load CGI_BIN from config
	}
	else if (reqType == GET) {
		/* TODO's handle response code */
		/*        check permissions for a certain file access */
		handle_static_request(client_fd, requested_file_path, buffer);
	}
	else if (reqType == DELETE) {
		handle_delete(client_fd, full_path, requested_file_path);
	}
}

void Server::handle_delete(int client_fd, std::string full_path, std::string file_path)
{
	// remove the first 4 chars from requested_file_path "ETE "
	full_path += file_path.substr(4);

	std::string response = "HTTP/1.1 ";

	// TODO check if full_path is a folder or an html file and dont remove it if so

	int ret = std::remove(full_path.c_str());
	if (ret != 0) {
		perror("remove");
		// TODO send error code
		response += "404 not found\r\n";
	}
	else {
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		response += "200 ok\r\n";
	}
	send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handle_file_request(int client_fd, const std::string &file_path)
{
	std::string full_path =
		"website" + file_path; // TODO use config root folder for corresponding server
	std::string file_content = readFileToString(full_path);
	std::string content_type = getContentType(file_path);

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type +
		"\r\nContent-Length: " + intToString(file_content.length()) + "\r\n\r\n" + file_content;

	send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handle_static_request(int client_fd,
								   const std::string &requested_file_path,
								   const char *buffer)
{
	std::string full_path = "website" + requested_file_path;
	struct stat path_stat;

	if (get_http_method(buffer) == GET && autoindex == false) {
		if (stat(full_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
			// It's a directory, generate directory listing for the requested path
			std::string dir_list = generateDirectoryListing(full_path);

			// Send HTTP response with the directory listing
			std::string response = "HTTP/1.1 200 OK\r\n";
			response +=
				"Content-Type: text/html\r\nContent-Length: " + intToString(dir_list.size()) +
				"\r\n\r\n" + dir_list;
			send(client_fd, response.c_str(), response.size(), 0);
		}
		else {
			handle_file_request(client_fd, requested_file_path);
		}
	}
	else if (get_http_method(buffer) == GET && autoindex == true) {
		// forward to index.html if autoindex is enabled
		if (autoindex == true && requested_file_path == "/")
			handle_file_request(client_fd, "/index.html");
		else
			handle_file_request(client_fd, requested_file_path);
	}
}

void Server::handle_write(int client_fd)
{
	char buffer[BUFFER_SIZE];
	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client_fd, buffer, BUFFER_SIZE, 0);
	if (is_file_upload_request(buffer)) {
		if (size == -1) {
			perror("recv");
			return;
		}

		std::string request(buffer);

		FileUploader uploader;
		std::size_t content_length = extract_content_length(buffer);
		std::string filename = extract_filename_from_request(buffer);

		// we must extract the first part of the request body in between the boundary
		// to pass it to the file upload which will further extract the file content
		// but the first part of the request body is not in the buffer if we dont read it

		std::string content = extract_content_body(request.c_str());

		uploader.handle_file_upload(client_fd, filename, content_length, content.c_str());
	}
}

void Server::handle_cgi_request(int client_fd, const std::string &cgi_script_path)
{
	int forked = fork();
	if (forked == -1) {
		perror("fork");
		return;
	}

	if (forked == 0) {
		Cgi cgi;
		std::string cgi_response = cgi.run(cgi_script_path);

		// Construct HTTP response
		std::string response = "HTTP/1.1 200 OK\r\n";
		response +=
			"Content-Type: text/html\r\nContent-Length: " + intToString(cgi_response.length()) +
			"\r\n\r\n" + cgi_response + "\r\n";

		// Send response to client
		send(client_fd, response.c_str(), response.size(), 0);
		exit(0);
	}
	else {
		close(client_fd);
		return;
	}
}

void Server::stop(int signal)
{
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

void Server::start()
{
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);

	start_listen();
	await_connections();
}
