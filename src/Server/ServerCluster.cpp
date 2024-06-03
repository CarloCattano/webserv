#include "./ServerCluster.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../Cgi/Cgi.hpp"
#include "../Utils/FileUpload.hpp"
#include "../Utils/utils.hpp"

const int MAX_EVENTS = 42;

ServerCluster::ServerCluster(std::vector<Server> &servers)
{
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

int ServerCluster::accept_new_connection(int server_fd)
{
	int client_fd = accept(server_fd, NULL, NULL);

	if (client_fd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	return (client_fd);
}

void ServerCluster::handle_new_client_connection(int server_fd)
{
	int client_fd = accept_new_connection(server_fd);

	Client *client = new Client(client_fd, &_server_map[server_fd], _epoll_fd);

	_client_map[client_fd] = *client;
}

void ServerCluster::close_client(int fd)
{
	_client_map.erase(fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

void ServerCluster::await_connections()
{
	struct epoll_event events[MAX_EVENTS];
	int num_events;

	while (1) {
		num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, 5000);
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
			}
			else {
				Client &client = _client_map[event_fd];

				if (events[i].events & EPOLLIN)
					handle_request(client);
				if (events[i].events & EPOLLOUT) {
					handle_response(client);
				}

				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
					close_client(event_fd);
				}
			}
			/* log_open_clients(_client_map); */
		}
	}
}

void ServerCluster::switch_poll(int client_fd, uint32_t events)
{
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

void ServerCluster::handle_request(Client &client)
{
	unsigned int buffer_size = 4096;
	char buffer[buffer_size];

	int bytes_read = recv(client.getFd(), buffer, buffer_size, 0);

	if (bytes_read == -1) {
		perror("recv");
		close_client(client.getFd());
		return;
	}

	client.appendRequestString(std::string(buffer, bytes_read));

	size_t end_of_header = client.getRequest().request.find("\r\n\r\n");
	if (end_of_header == std::string::npos)
		return;
	if (!client.getRequest().finishedHead)
		client.parseHead();
	client.parseBody();
	if (!client.getRequest().finished)
		return;

	if (client.getRequest().method == "GET") {
		handle_get_request(client);
	}
	else if (client.getRequest().method == "POST") {
		handle_post_request(client);
	}
	else if (client.getRequest().method == "DELETE")
		handle_delete_request(client);
	else {
		client.sendErrorPage(405);
		close_client(client.getFd());
	}

	if (bytes_read == 0)
		close_client(client.getFd());

	switch_poll(client.getFd(), EPOLLOUT);
}

void ServerCluster::handle_response(Client &client)
{
	Response response = client.getResponse();

	std::string response_string = client.responseToString();
	client.setResponseSize(response_string.size());
	client.setSentBytes(client.getSentBytes() + send(client.getFd(), response_string.c_str(), 4096, 0));

	if (client.getSentBytes() >= response_string.size()) {
		close_client(client.getFd());
	}
}

bool ServerCluster::allowed_in_path(const std::string &file_path, Client &client)
{
	if (file_path.find(client.getServer()->getRoot()) == std::string::npos)
		return false;
	return true;
}

void ServerCluster::handle_get_request(Client &client)
{
	Server *server = client.getServer();

	std::string full_path = "." + server->getRoot() + client.getRequest().uri;
	if (client.getRequest().body.size() > static_cast<unsigned long>(server->getClientMaxBodySize())) {
		log("Body size is too big");
		client.sendErrorPage(413);
		return;
	}

	if (server->getAutoindex() == false) {
		if (isFolder(full_path) == true) {
			std::string dir_list = generateDirectoryListing(full_path);
			client.setResponseBody(dir_list);
			client.setResponseStatusCode(200);
			client.addResponseHeader("Content-Type", "text/html");
			client.addResponseHeader("Content-Length", intToString(dir_list.size()));
			client.addResponseHeader("Connection", "keep-alive");
		}
		else {
			if (!isFile(full_path)) {
				client.sendErrorPage(404);
				return;
			}
			// TODO - check image bug
			std::string file_content = readFileToString(full_path);
			std::string content_type = getContentType(full_path);
			client.setResponseBody(file_content);
			client.addResponseHeader("Content-Type", content_type);
			client.addResponseHeader("Content-Length", intToString(file_content.size()));
			client.setResponseStatusCode(200);
			client.addResponseHeader("Connection", "keep-alive");
		}
	}
	else {
		std::string file_content = readFileToString(full_path);
		std::string content_type = getContentType(full_path);

		if (isFolder(full_path) && client.getRequest().uri != "/") {
			std::string dir_list = generateDirectoryListing(full_path);
			client.setResponseBody(dir_list);
			client.setResponseStatusCode(200);
			client.addResponseHeader("Content-Type", "text/html");
			client.addResponseHeader("Content-Length", intToString(dir_list.size()));
			client.addResponseHeader("Connection", "keep-alive");
		}
		else {
			std::cout << client.getRequest().uri << std::endl;
			if (client.getRequest().uri == "/") {
				full_path += "index.html";
				file_content = readFileToString(full_path);
				content_type = getContentType(full_path);

				client.addResponseHeader("Content-Type", content_type);
				client.setResponseBody(file_content);
				client.addResponseHeader("Content-Length", intToString(file_content.size()));
			}
			else {
				if (!isFile(full_path)) {
					client.sendErrorPage(404);
					return;
				}
				file_content = readFileToString(full_path);
				client.setResponseBody(file_content);
				client.addResponseHeader("Content-Type", content_type); // TODO BUG with images
				client.addResponseHeader("Content-Length", intToString(file_content.size()));
			}
			client.setResponseStatusCode(200);
			client.addResponseHeader("Connection", "keep-alive");
		}
	}
}

void ServerCluster::handle_post_request(Client &client)
{
	std::string full_path = "." + client.getServer()->getRoot() + client.getRequest().uri;

	if (client.getRequest().body.size() > static_cast<unsigned long>(client.getServer()->getClientMaxBodySize())) {
		log("Body size is too big");
		client.sendErrorPage(413);
		close_client(client.getFd());
		return;
	}
	else {
		if (client.getRequest().uri == "/upload") {
			handle_file_upload(client);
			/* close_client(client.getFd()); */
		}
		/* std::string cgi_script_path = "." + client.getServer()->getCgiPath() + client.getRequest().uri; */
		/* handle_cgi_request(client, cgi_script_path); */
	}
	client.setResponseStatusCode(200);
	client.addResponseHeader("Content-Type", "text/html");
	client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
}

void ServerCluster::handle_file_upload(Client &client)
{
	std::string headers = client.getRequest().request.substr(0, client.getRequest().request.find("\r\n\r\n"));
	std::string body = client.getRequest().body;

	std::string boundary = extract_boundary(headers);
	FileUploader fileUploader;
	MultipartFormData formData = fileUploader.parse_multipart_form_data(boundary, body);

	if (formData.fileName.empty()) {
		client.sendErrorPage(400);
		return;
	}

	if (formData.fileContent.size() > static_cast<unsigned long>(client.getServer()->getClientMaxBodySize())) {
		log("Body size is too big");
		client.sendErrorPage(413);
		return;
	}

	if (formData.fileContent.empty()) {
		client.sendErrorPage(400);
		return;
	}

	std::string upload_path = "." + client.getServer()->getRoot() + "/upload/" + formData.fileName;
	std::ofstream outFile(upload_path.c_str(), std::ios::binary);

	outFile.write(&formData.fileContent[0], formData.fileContent.size());
	outFile.close();

	client.setResponseStatusCode(200);
	client.setResponseBody("File was uploaded successfully");
	client.addResponseHeader("Content-Type", "text/html");
	client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
	client.addResponseHeader("Connection", "keep-alive");
}

void ServerCluster::handle_delete_request(Client &client)
{
	Response response;

	std::string full_path = "." + client.getServer()->getRoot() + "/upload" + client.getRequest().uri;

	int is_allowed = allowed_in_path(full_path, client);

	if (isFolder(full_path)) {
		client.sendErrorPage(403);
	}
	else if (is_allowed == true && isFile(full_path) == true) {
		if (std::remove(full_path.c_str()) == 0) {
		};
		std::cout << "file " << full_path.c_str() << " was deleted from the server" << std::endl;
		client.setResponseStatusCode(200);
		client.setResponseBody("File was deleted successfully");
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Content-Length", intToString(client.getSentBytes()));
	}
	else {
		client.sendErrorPage(404);
	}
}

std::string ServerCluster::extract_boundary(const std::string &headers)
{
	std::string boundary;
	size_t boundaryPos = headers.find("boundary=");
	if (boundaryPos != std::string::npos) {
		boundary = headers.substr(boundaryPos + 9);
		size_t endPos = boundary.find("\r\n");
		if (endPos != std::string::npos) {
			boundary = boundary.substr(0, endPos);
		}
	}
	return boundary;
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
