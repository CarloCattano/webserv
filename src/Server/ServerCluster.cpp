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

std::string CGI_BIN =
	get_current_dir() + "/www/website1/cgi-bin/" + "test.py"; // TODO load from config

ServerCluster::ServerCluster() {}

ServerCluster::ServerCluster(std::vector<Server> servers) : _servers(servers) {
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
				int client_fd = event_fd;
				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
					continue;
				}
				if (events[i].events & EPOLLIN) {
					handle_request(client_fd);
				}
				if (events[i].events & EPOLLOUT) {
					handle_write(client_fd);
				}
				close(client_fd);
				epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
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

void ServerCluster::handle_request(int client_fd) {
	char buffer[BUFFER_SIZE];

	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	switch_poll(client_fd, EPOLLIN);
	int size = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (size == -1) {
		return;
	}

	if (size == -1) {
		perror("recv");
		return;
	}

	// check content lentgh
	std::size_t content_length = extract_content_length(buffer);
	// TODO get the Server configs in another place in the object
	Server *server = get_server_by_client_fd(client_fd);

	if (!server)
		return;
	// ----------------------------------------------------------

	if (static_cast<int>(content_length) > std::atoi(server->_client_max_body_size.c_str())) {
		Response response;
		response.ErrorResponse(client_fd, 413);
		return;
	}

	HttpMethod reqType = get_http_method(buffer);

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("www/website1" + requested_file_path);
	std::string content_type = getContentType(requested_file_path);
	std::string full_path = "www/website1/";

	if (reqType == GET) {
		/* TODO's handle response code */
		/*        check permissions for a certain file access */
		handle_get_request(client_fd, requested_file_path);
	}
	if (reqType == DELETE) {
		handle_delete_request(client_fd, full_path, requested_file_path);
	} else if (requested_file_path.find(".py") != std::string::npos && reqType == POST) {
		handle_cgi_request(client_fd, CGI_BIN); // TODO load CGI_BIN from config
	}

	else {
		Response response;
		response.ErrorResponse(client_fd, 405);
	}
}

void ServerCluster::handle_delete_request(int client_fd, std::string full_path,
										  std::string file_path) {
	// remove the first 4 chars from requested_file_path "ETE "
	full_path += file_path.substr(4);

	std::string response = "HTTP/1.1 ";

	// TODO check if full_path is a folder or an html file and dont remove it if so

	int ret = std::remove(full_path.c_str());
	if (ret != 0) {
		perror("remove");
		// TODO send error code
		response += "404 not found\r\n";
	} else {
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		response += "200 ok\r\n";
	}
	send(client_fd, response.c_str(), response.size(), 0);
	// TODO - check return value of send
}

void ServerCluster::handle_file_request(int client_fd, const std::string &file_path) {
	std::string full_path =
		"www/website1" + file_path; // TODO use config root folder for corresponding server
	std::string file_content = readFileToString(full_path);
	std::string content_type = getContentType(file_path);

	Response response;
	if (file_content.empty()) {
		response.ErrorResponse(client_fd, 404);
		return;
	}
	switch_poll(client_fd, EPOLLOUT);
	response.setStatusCode(200);
	response.setHeader("Connection", "keep-alive");
	response.setHeader("Content-Type", content_type);
	response.setHeader("Content-Length", intToString(file_content.length()));
	response.setBody(file_content);
	response.respond(client_fd, _epoll_fd);
}

Server *ServerCluster::get_server_by_client_fd(int client_fd) {
	if (!_client_fd_to_server_map.count(client_fd)) {
		perror("No matching server for client fd");
		return NULL;
	}
	return (&_client_fd_to_server_map[client_fd]);
}

void ServerCluster::handle_get_request(int client_fd, const std::string &requested_file_path) {
	Server *server = get_server_by_client_fd(client_fd);
	if (!server)
		return;

	std::string full_path = "www/website1" + requested_file_path;
	struct stat path_stat;

	// HttpMethod reqType = get_http_method(buffer);

	switch_poll(client_fd, EPOLLOUT);

	Response response;

	if (server->_autoindex == false) {
		if (stat(full_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
			// It's a directory, generate directory listing for the requested path
			std::string dir_list = generateDirectoryListing(full_path);

			response.setStatusCode(200);
			response.setHeader("Connection", "keep-alive");
			response.setHeader("Content-Type", "text/html");
			response.setHeader("Content-Length", intToString(dir_list.size()));
			response.setBody(dir_list);
			response.respond(client_fd, _epoll_fd);
		} else {
			handle_file_request(client_fd, requested_file_path);
		}
	} else {
		// forward to index.html if autoindex is enabled
		if (requested_file_path == "/")
			handle_file_request(client_fd, "/index.html");
		else
			handle_file_request(client_fd, requested_file_path);
	}
}

void ServerCluster::handle_write(int client_fd) {
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

		switch_poll(client_fd, EPOLLIN);
		uploader.handle_file_upload(client_fd, filename, content_length, content.c_str());
		// TODO - craft a response to the client
	}
}

void ServerCluster::handle_cgi_request(int client_fd, const std::string &cgi_script_path) {

	Response response;
	switch_poll(client_fd, EPOLLOUT);

	int forked = fork();
	if (forked == -1) {
		perror("fork");
		return;
	}

	if (forked == 0) {
		Cgi cgi;
		std::string cgi_response = cgi.run(cgi_script_path);

		if (!cgi_response.empty()) {
			response.setStatusCode(200);
			response.setHeader("Connection", "keep-alive");
			response.setHeader("Content-Type", "text/html");
			response.setHeader("Content-Length", intToString(cgi_response.length()));
			response.setBody(cgi_response);
			response.respond(client_fd, _epoll_fd);
			close(client_fd);
			exit(0);
		} else {
			response.ErrorResponse(client_fd, 500);
			exit(0);
		}
	}
	switch_poll(client_fd, EPOLLIN);
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
