#include <iostream>
#include <sys/wait.h>
#include "../Cgi/Cgi.hpp"
#include "../Utils/utils.hpp"
#include "./ServerCluster.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/stat.h>

const int MAX_EVENTS = 100;
const int BACKLOG = 20;
const int BUFFER_SIZE = 1024;
const bool autoindex = false; // TODO load from config

std::string CGI_BIN = get_current_dir() + "/website/cgi-bin/" + "hello.py"; // TODO load from config

ServerCluster::ServerCluster() {}
ServerCluster::ServerCluster(std::vector<Server> servers) : _servers(servers) {}

void    ServerCluster::setupCluster() {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

    for (size_t i = 0; i < _servers.size(); i++) {
        int socket_fd = _servers[i].getSocketFd();
    
        _server_map[socket_fd] = _servers[i];

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = socket_fd;
        
        std::cout << "here" << std::endl;
    
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
            perror("epoll_ctl");
            exit(EXIT_FAILURE);
        }
    }
}

void    ServerCluster::await_connections()
{
	while (1) { // TODO add a flag to run the server
		struct epoll_event events[MAX_EVENTS];

		int num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);

		if (num_events == -1) {
			continue;
		}

		for (int i = 0; i < num_events; ++i) {
			if (_server_map.count(events[i].data.fd)) {
				int client_fd = accept(events[i].data.fd, NULL, NULL);

				if (client_fd == -1) {
					perror("accept");
					continue;
				}

	            struct epoll_event ev;
				ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
				ev.data.fd = client_fd;

				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
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
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
				else if (events[i].events & EPOLLOUT) {
					handle_write(client_fd);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
				else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
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

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type +
		"\r\nContent-Length: " + intToString(file_content.length()) + "\r\n\r\n" + file_content;

	send(client_fd, response.c_str(), response.size(), 0);
}

void ServerCluster::handle_static_request(int client_fd,
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