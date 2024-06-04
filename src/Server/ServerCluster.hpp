#pragma once

#include <map>
#include <sys/epoll.h>
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../Client/Client.hpp"
#include "../Cgi/Cgi.hpp"
#include "./Server.hpp"
#include "../Utils/FileUpload.hpp"
#include "../Utils/utils.hpp"

class ServerCluster {
private:
	std::map<int, Server> _server_map;
	std::map<int, Client> _client_map;

	int _epoll_fd;

public:
	ServerCluster(std::vector<Server> &servers);
	~ServerCluster();
	void start();
	void await_connections();
	void close_client(int client_fd);
	static void stop(int signal);
	void handle_new_client_connection(int server_fd);
	int accept_new_connection(int server_fd);

	void handle_request(Client &client);
	void handle_response(Client &client);

	// void handle_cgi_request(const Client &client, const std::string &cgi_script_path);
	void handle_get_request(Client &client);
	void handle_post_request(Client &client);
	void handle_delete_request(Client &client);
	Client get_client_obj(epoll_event &event);


	/* FileUploader uploader; */
	void handle_file_upload(Client &client);
	std::string extract_boundary(const std::string &headers);
	void switch_poll(int client_fd, uint32_t events);


	bool allowed_in_path(const std::string &file_path, Client &client);
};