#include "./ServerCluster.hpp"
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
#include "../Response/Response.hpp"
#include "../Utils/utils.hpp"

const int MAX_EVENTS = 100;
const int BUFFER_SIZE = 1024;
const bool autoindex = true; // TODO load from config

std::string CGI_BIN = get_current_dir() + "/website/cgi-bin/" + "hello.py"; // TODO load from config

ServerCluster::ServerCluster() {}
ServerCluster::ServerCluster(std::vector<Server> servers) : _servers(servers) {
	this->setupCluster();
}

void ServerCluster::setupCluster()
{
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		perror("epoll_create1");
	}

	for (size_t i = 0; i < _servers.size(); i++) {
		int socket_fd = _servers[i].getSocketFd();
		std::cout << socket_fd << std::endl;

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

void	ServerCluster::new_connection(int fd) {
	int client_fd = accept(fd, NULL, NULL);

	if (client_fd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
	ev.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
}

void ServerCluster::await_connections()
{
	while (1) { // TODO add a flag to run the server
		struct epoll_event events[MAX_EVENTS];

		int num_events;

		do {
			num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, 10);
		} while (num_events == -1);
		if (num_events == -1) {
			perror("epoll_wait");
		}

		for (int i = 0; i < num_events; i++) {
			if (_server_map.count(events[i].data.fd)) {
				int client_fd = accept(events[i].data.fd, NULL, NULL);

				// if the client_fd is -1, then the accept failed
				// and we should continue to the next event
				if (client_fd == -1) {
					perror("accept");
					continue; // continue to the next event
				}

				struct epoll_event ev;
				ev.events = EPOLLIN | EPOLLOUT; //| EPOLLERR | EPOLLHUP;
				ev.data.fd = client_fd;

				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
					perror("epoll_ctl");
				}
			}
			else {
				int client_fd = events[i].data.fd;
				if (client_fd == -1) {
					perror("events[i].data.fd");
					continue;
				}

				if (events[i].events & EPOLLIN) {
					handle_request(client_fd);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
				/* else if (events[i].events & EPOLLOUT) { */
				/* 	handle_write(client_fd); */
				/* 	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); */
				/* 	close(client_fd); */
				/* } */
				/* else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) { */
				/* 	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); */
				/* 	close(client_fd); */
				/* } */
			}
		}
	}
}

void ServerCluster::handle_request(int client_fd)
{
	char buffer[BUFFER_SIZE];

	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	int size = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
	if (size == -1) {
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

	if (reqType == GET) {
		/* TODO's handle response code */
		/*        check permissions for a certain file access */
		handle_static_request(client_fd, requested_file_path, buffer);
	}
	if (reqType == DELETE) {
		handle_delete(client_fd, full_path, requested_file_path);
	}
	else if (requested_file_path.find(".py") != std::string::npos && reqType == POST) {
		handle_cgi_request(client_fd, CGI_BIN); // TODO load CGI_BIN from config
	}

	// no match for the request
	else {
		Response response;
		response.setStatusCode(404);
		response.setHeader("Connection", "keep-alive");
		response.setHeader("Content-Type", "text/html");
		response.setHeader("Content-Length", "0");
		response.respond(client_fd);
	}
}

void ServerCluster::handle_delete(int client_fd, std::string full_path, std::string file_path)
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

void ServerCluster::handle_file_request(int client_fd, const std::string &file_path)
{
	std::string full_path =
		"website" + file_path; // TODO use config root folder for corresponding server
	std::string file_content = readFileToString(full_path);
	std::string content_type = getContentType(file_path);

	Response response;

	response.setStatusCode(200);
	response.setHeader("Connection", "keep-alive");
	response.setHeader("Content-Type", content_type);
	response.setHeader("Content-Length", intToString(file_content.length()));
	response.setBody(file_content);
	response.respond(client_fd);
}

void ServerCluster::handle_static_request(int client_fd,
										  const std::string &requested_file_path,
										  const char *buffer)
{
	std::string full_path = "website" + requested_file_path;
	struct stat path_stat;

	HttpMethod reqType = get_http_method(buffer);

	Response response;

	if (reqType == GET && autoindex == false) {
		if (stat(full_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
			// It's a directory, generate directory listing for the requested path
			std::string dir_list = generateDirectoryListing(full_path);

			response.setStatusCode(200);
			response.setHeader("Connection", "keep-alive");
			response.setHeader("Content-Type", "text/html");
			response.setHeader("Content-Length", intToString(dir_list.size()));
			response.setBody(dir_list);
			response.respond(client_fd);
		}
		else {
			handle_file_request(client_fd, requested_file_path);
		}
	}
	else if (reqType == GET && autoindex == true) {
		// forward to index.html if autoindex is enabled
		if (autoindex == true && requested_file_path == "/")
			handle_file_request(client_fd, "/index.html");
		else
			handle_file_request(client_fd, requested_file_path);
	}
}

void ServerCluster::handle_write(int client_fd)
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

void ServerCluster::handle_cgi_request(int client_fd, const std::string &cgi_script_path)
{
	Response response;

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
			response.respond(client_fd);
			close(client_fd);
			exit(0);
		}
		else {
			response.setStatusCode(500);
			response.setHeader("Connection", "keep-alive");
			response.setHeader("Content-Type", "text/html");
			response.setHeader("Content-Length", "0");
			response.respond(client_fd);
		}
	}
}

void ServerCluster::stop(int signal)
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

void ServerCluster::start()
{
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);
	ServerCluster::await_connections();
}

ServerCluster::~ServerCluster()
{
	// for (size_t i = 0; i < _servers.size(); i++) {
	//     close(_servers[i].getSocketFd());
	// }
}
