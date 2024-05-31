#include "./ServerCluster.hpp"
#include "../Cgi/Cgi.hpp"
#include "../Response/Response.hpp"
#include "../Utils/utils.hpp"
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/wait.h>

const int MAX_EVENTS = 42;
const int BUFFER_SIZE = 1024;

ServerCluster::ServerCluster(std::vector<Server> &servers) : _servers(servers) {
	this->setupCluster();
}

void ServerCluster::setupCluster() {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		perror("epoll_create1");
	}
	for (size_t i = 0; i < _servers.size(); i++) {
		int socket_fd = _servers[i].getSocketFd();

		_server_map[socket_fd] = _servers[i];

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
			perror("epoll_ctl");
			exit(EXIT_FAILURE);
		}
	}
}

int ServerCluster::accept_new_connection(int server_fd) {
	int client_fd = accept(server_fd, NULL, NULL);

	if (client_fd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	return (client_fd);
}

void ServerCluster::add_client_fd_to_epoll(int client_fd) {
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
}

void ServerCluster::handle_new_client_connection(int server_fd) {
	int client_fd = accept_new_connection(server_fd);
	add_client_fd_to_epoll(client_fd);
	_client_fd_to_server_map[client_fd] = _server_map[server_fd];
}

Client ServerCluster::get_client_obj(const int &client_fd) {
	Client client;

	client.fd = client_fd;

	if (!_client_fd_to_server_map.count(client_fd))
		throw std::runtime_error("No matching server for client fd");
	client.server = &_client_fd_to_server_map[client_fd];
	return (client);
}

void ServerCluster::await_connections() {
	struct epoll_event events[MAX_EVENTS];
	int num_events;

	while (1) { // TODO add a flag to run the server
		num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, 500);
		if (num_events == -1)
			continue;

		for (int i = 0; i < num_events; i++) {

			int event_fd = events[i].data.fd;
			if (event_fd == -1) {
				perror("events[i].data.fd");
				continue;
			}

			if (_server_map.count(event_fd)) {
				handle_new_client_connection(event_fd);

			} else {
				Client client = get_client_obj(event_fd);

				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.fd, NULL);
					log("Error or hangup");
					close(client.fd);
					continue;
				}
				if (events[i].events & EPOLLIN) {
					log("Handling request");
					handle_request(client);
				}
				if (events[i].events & EPOLLOUT) {
					log("Handling write");
					/* handle_write(client); */
				}
				close(client.fd);
				epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.fd, NULL);
			}
		}
	}
}

void ServerCluster::switch_poll(int client_fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = client_fd;

	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		perror("epoll_ctl");
	}
}

void ServerCluster::logConfig(const Client &client) {

	log("Server configuration:");
	log("Server port: " + intToString(client.server->getPort()));
	log("Server root: " + client.server->getRoot());
	log("Server autoindex: " +
		static_cast<std::string>(client.server->getAutoindex() ? "on" : "off"));
	log("Server default server: " +
		static_cast<std::string>(client.server->getDefaultServer() ? "on" : "off"));
	log("Server client max body size: " + intToString(client.server->getClientMaxBodySize()));
	log("Server CGI path: " + client.server->getCgiPath());
	log("Server CGI extension: " + client.server->getCgiExtension());
}

void ServerCluster::handle_request(const Client &client) {
	char buffer[BUFFER_SIZE];

	if (client.fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client.fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (size == -1) {
		return;
	}

	if (size == -1) {
		perror("recv");
		return;
	}

	std::size_t content_length = extract_content_length(buffer);

	logConfig(client);

	if (static_cast<long long>(content_length) > client.server->getClientMaxBodySize()) {
		Response response;
		response.ErrorResponse(client.fd, 413);
		return;
	}

	HttpMethod reqType = get_http_method(buffer);

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString(client.server->getRoot() + requested_file_path);
	std::string content_type = getContentType(requested_file_path);
	std::string full_path = client.server->getRoot() + requested_file_path;

	if (reqType == GET) {
		/* TODO's handle response code */
		/*        check permissions for a certain file access */
		handle_get_request(client, requested_file_path);
	} else if (reqType == DELETE) {
		handle_delete_request(client, full_path, requested_file_path);
	} else if (requested_file_path.find(client.server->getCgiExtension()) != std::string::npos &&
			   (reqType == POST || reqType == GET)) {
		/* handle_cgi_request(client, client.server->getCgiPath()); */
		log("Insert CGI handler here .....\n .... \n");
	}
}

void ServerCluster::handle_delete_request(const Client &client, std::string full_path,
										  std::string file_path) {

	full_path += file_path;
	std::string response = "HTTP/1.1 ";

	// TODO check if full_path is a folder or an html file and dont remove it if so

	/* int ret = std::remove(full_path.c_str()); */
	int ret = -1;
	if (ret != 0) {
		perror("remove");
		// TODO send error code
		response += "404 not found\r\n";
	} else {
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		response += "200 ok\r\n";
	}
	send(client.fd, response.c_str(), response.size(), 0);
	// TODO - check return value of send
}

void ServerCluster::handle_file_request(const Client &client, const std::string &file_path) {
	std::string full_path = client.server->getRoot() + file_path;
	std::string file_content = readFileToString("." + full_path);
	std::string content_type = getContentType("." + file_path);

	Response response;

	if (file_content.empty()) {
		response.ErrorResponse(client.fd, 404);
		return;
	}

	if (full_path.find(client.server->getRoot()) == std::string::npos) {
		response.ErrorResponse(client.fd, 403);
		return;
	}

	switch_poll(client.fd, EPOLLOUT);
	response.setStatusCode(200);
	response.setHeader("Connection", "keep-alive");
	response.setHeader("Content-Type", content_type);
	response.setHeader("Content-Length", intToString(file_content.length()));
	response.setBody(file_content);
	response.respond(client.fd, _epoll_fd);
}

void ServerCluster::handle_get_request(const Client &client,
									   const std::string &requested_file_path) {

	std::string full_path = client.server->getRoot() + requested_file_path;
	struct stat path_stat;

	switch_poll(client.fd, EPOLLOUT);

	Response response;

	if (client.server->getAutoindex() == false && stat(full_path.c_str(), &path_stat) == 0 &&
		S_ISDIR(path_stat.st_mode)) {
		// It's a directory, generate directory listing for the requested path
		std::string dir_list = generateDirectoryListing(full_path);

		response.setStatusCode(200);
		response.setHeader("Connection", "keep-alive");
		response.setHeader("Content-Type", "text/html");
		response.setHeader("Content-Length", intToString(dir_list.size()));
		response.setBody(dir_list);
		response.respond(client.fd, _epoll_fd);
	} else
		handle_file_request(client,
							requested_file_path == "/" ? "/index.html" : requested_file_path);
}

void ServerCluster::handle_write(const Client &client) {
	char buffer[BUFFER_SIZE];
	if (client.fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client.fd, buffer, BUFFER_SIZE, 0);
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

		switch_poll(client.fd, EPOLLIN);
		uploader.handle_file_upload(client.fd, filename, content_length, content.c_str());
		// TODO - craft a response to the client
	}
}

void ServerCluster::handle_cgi_request(const Client &client, const std::string &cgi_script_path) {

	// build the path to the cgi script by looking at the request requested file path
	// and compare against client.server->getCgiPath() to get verify that we have permission to
	// execute the script

	char buffer[BUFFER_SIZE];
	int size = recv(client.fd, buffer, BUFFER_SIZE, 0);
	if (size == -1) {
		perror("recv");
		return;
	}

	std::string script_path = client.server->getRoot() + cgi_script_path;
	log("script path: " + script_path);
}

void ServerCluster::stop(int signal) {
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

/* takes care of the signal when a child process is terminated
	and the parent process is not waiting for it
	so it doesn't become a zombie process */
void handleSigchild(int sig) {
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
}

void ServerCluster::start() {
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);
	ServerCluster::await_connections();
}

ServerCluster::~ServerCluster() {
	// for (size_t i = 0; i < _servers.size(); i++) {
	//     close(_servers[i].getSocketFd());
	// }
}
