#include "./ServerCluster.hpp"
#include "../Cgi/Cgi.hpp"
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
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

const int MAX_EVENTS = 42;
// const int BUFFER_SIZE = 1024;

ServerCluster::ServerCluster(std::vector<Server> &servers) {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
		perror("epoll_create1");

	for (size_t i = 0; i < servers.size(); i++) {
		int socket_fd = servers[i].getSocketFd();

		_server_map[socket_fd] = servers[i];

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

void ServerCluster::handle_new_client_connection(int server_fd) {

	int client_fd = accept_new_connection(server_fd);

	Client *client = new Client(client_fd, &_server_map[server_fd], _epoll_fd);

	_client_map[client_fd] = *client;
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
				Client &client = _client_map[event_fd];

				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
					_client_map.erase(event_fd);
					close(event_fd);

					continue;
				}
				if (events[i].events & EPOLLIN)
					handle_request(client);
				if (events[i].events & EPOLLOUT) {
					handle_response(client);
				}
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

void ServerCluster::handle_request(Client &client) {
	//handle request
	char buffer[4096];

	int bytes_read = recv(client.getFd(), buffer, 4096, 0);

	if (bytes_read == -1) {}
	//error

	client.appendRequestString(std::string(buffer, bytes_read));

	size_t end_of_header = client.getRequest().request.find("\r\n\r\n");
	if (end_of_header == std::string::npos)
		return;
	if (!client.getRequest().finishedHead)
		client.parseHead();
	client.parseBody();
	if (!client.getRequest().finished)
		return;

	log(" METHOD ::::|" + client.getRequest().method + "|");

	if (client.getRequest().method == "GET") {
		log("GET request");
		handle_get_request(client);
	} else if (client.getRequest().method == "POST") {
		// handle_post_request(client, client.getRequest().uri);
	} else if (client.getRequest().method == "DELETE") {}
		// handle_delete_request(client, client.getRequest().uri);

	switch_poll(client.getFd(), EPOLLOUT);
	
}

//handle response
void ServerCluster::handle_response(Client &client) {
	std::cout << "handle response" << std::endl;
	Response response = client.getResponse();
	
	std::string response_string = client.responseToString();
	client.setResponseSize(response_string.size());
	client.setSentBytes( client.getSentBytes() + send(client.getFd(), response_string.c_str(), 4096, 0));

	if (client.getSentBytes() >= response_string.size()) {
		
	}

}

int ServerCluster::allowed_in_path(const std::string &file_path, Client &client) {

	if (file_path.find(client.getServer()->getRoot()) == std::string::npos)
		return -1;

	struct stat buffer;
	if (stat(file_path.c_str(), &buffer) != 0)
		return -2;
	if (S_ISDIR(buffer.st_mode))
		return 2;
	return 0;
}

// void ServerCluster::handle_delete_request(const Client &client, std::string requested_file_path) {

// 	Response response;

// 	std::string full_path = "." + client.server->getRoot() + "/upload" + requested_file_path;

// 	if (allowed_in_path(full_path, const_cast<Client &>(client))) {
// 		response.ErrorResponse(client.fd, 403);
// 		return;
// 	}

// 	int ret = std::remove(full_path.c_str());
// 	if (ret != 0) {
// 		perror("remove");
// 		response.ErrorResponse(client.fd, 404);
// 	} else {
// 		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
// 		response.setStatusCode(200);
// 		response.setBody("File was deleted successfully");
// 		response.respond(client.fd, _epoll_fd);
// 	}
// }

// void ServerCluster::handle_file_request(const Client &client, const std::string &file_path) {

// 	std::string full_path = client.server->getRoot() + file_path;
// 	std::string file_content = readFileToString("." + full_path);
// 	std::string content_type = getContentType("." + file_path);

// 	Response response;

// 	if (file_content.empty()) {
// 		response.ErrorResponse(client.fd, 404);
// 		return;
// 	}

// 	int is_allowed = allowed_in_path(full_path, const_cast<Client &>(client));

// 	if (is_allowed == -1) {
// 		response.ErrorResponse(client.fd, 403);
// 		return;
// 	}


// 	response.setStatusCode(200);
// 	response.setHeader("Connection", "keep-alive");
// 	response.setHeader("Content-Type", content_type);
// 	response.setHeader("Content-Length", intToString(file_content.length()));
// 	response.setBody(file_content);
// 	response.respond(client.fd, _epoll_fd);
//     close(client.fd);
// }

void ServerCluster::handle_get_request(Client &client) {
	std::cout << "handle get request" << std::endl;
	Server *server = client.getServer();

	std::string full_path = "." + server->getRoot() + client.getRequest().uri;

	if (server->getAutoindex() == false &&
		allowed_in_path(full_path, const_cast<Client &>(client)) == 2){
		std::string dir_list = generateDirectoryListing(full_path);
		client.setResponseBody(dir_list);
		client.setResponseStatusCode(200);
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Content-Length", intToString(dir_list.size()));
	}
	// else
	// 	handle_file_request(client,
	// 						requested_file_path == "/" ? "/index.html" : requested_file_path);
}

// void ServerCluster::handle_cgi_request(const Client &client, const std::string &cgi_script_path) {
// 	Cgi cgi;
// 	std::string cgi_res = cgi.run(cgi_script_path);
// 	Response response;

// 	if (cgi_res.empty()) {
// 		response.ErrorResponse(client.fd, 500);
// 		return;
// 	}

// 	response.setStatusCode(200);
// 	response.setHeader("Connection", "keep-alive");
// 	response.setHeader("Content-Type", "text/html");
// 	response.setHeader("Content-Length", intToString(cgi_res.size()));
// 	response.setBody(cgi_res);
// 	response.respond(client.fd, _epoll_fd);
// }

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
