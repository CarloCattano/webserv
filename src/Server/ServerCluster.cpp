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
#include <sys/wait.h>

const int MAX_EVENTS = 42;
// const int BUFFER_SIZE = 1024;

std::string CGI_BIN =
	get_current_dir() + "/www/website1/cgi-bin/" + "test.py"; // TODO load from config

ServerCluster::ServerCluster() {}

ServerCluster::ServerCluster(std::vector<Server> servers) {

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
				if (events[i].events & EPOLLIN) {
					handle_request(client);
				}
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
	std::cout << "handle request" << std::endl;
	// Response *response = &client.getResponse();
	
	// Request *request = &client.getRequest();

	this->switch_poll(client.getFd(), EPOLLOUT);	
	//get request

}

//handle response
void ServerCluster::handle_response(Client &client) {
	std::cout << "handle response" << std::endl;
	Response response = client.getResponse();
	
	std::string response_string = client.responseToString();
	client.setResponseSize(response_string.size());
	client.setSentBytes( client.getSentBytes() + send(client.getFd(), response_string.c_str(), 3, 0));

	if (client.getSentBytes() >= response_string.size()) {
		std::cout << "response: \nsize: "<< client.getResponseSize() << "\n" << response_string << std::endl;
		this->switch_poll(client.getFd(), EPOLLIN);
		exit(69);
	}

}

// void ServerCluster::handle_get_request(const Client &client,
// 									   const std::string &requested_file_path) {

// 	switch_poll(client.fd, EPOLLOUT);

// 	Response response;

// 	if (client.server->getAutoindex() == false && stat(full_path.c_str(), &path_stat) == 0 &&
// 		S_ISDIR(path_stat.st_mode)) {
// 		// It's a directory, generate directory listing for the requested path
// 		std::string dir_list = generateDirectoryListing(full_path);

// 		response.setStatusCode(200);
// 		response.addHeader("Content-Type", "text/html");
// 		response.setBody(dir_list);
// 	} else
// 		handle_file_request(client,
// 							requested_file_path == "/" ? "/index.html" : requested_file_path);
// }


// void ServerCluster::handle_cgi_request(const Client &client, const std::string &cgi_script_path) {

// 	Response response;
// 	switch_poll(client.fd, EPOLLOUT);

// 	int forked = fork();
// 	if (forked == -1) {
// 		perror("fork");
// 		return;
// 	}

// 	if (forked == 0) {
// 		Cgi cgi;
// 		std::string cgi_response = cgi.run(cgi_script_path);

// 		if (!cgi_response.empty()) {
// 			response.setStatusCode(200);
// 			response.setHeader("Connection", "keep-alive");
// 			response.setHeader("Content-Type", "text/html");
// 			response.setHeader("Content-Length", intToString(cgi_response.length()));
// 			response.setBody(cgi_response);
// 			response.respond(client.fd, _epoll_fd);
// 			close(client.fd);
// 			exit(0);
// 		} else {
// 			response.ErrorResponse(client.fd, 500);
// 			exit(0);
// 		}
// 	}
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
