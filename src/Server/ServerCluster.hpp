#pragma once

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
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
#include "../Client/Client.hpp"
#include "../Utils/utils.hpp"
#include "./Server.hpp"

class ServerCluster {
private:
	std::map<int, Server>		_server_map;
	std::map<int, Client> 		_client_map;
	std::map<int, time_t> 		_client_start_time_map;
	std::map<int, int> 			_pipeFd_clientFd_map;
	std::map<int, std::string> 	_cgi_response_map;
	int 						_epoll_fd;

	void handle_pipe_event(int pipe_fd);

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
	void handle_get_request(Client &client, Server *server);
	void handle_post_request(Client &client, Server *server);
	void handle_delete_request(Client &client, Server *server);
	Client get_client_obj(epoll_event &event);


	/* FileUploader uploader; */
	void handle_file_upload(Client &client);
	std::string extract_boundary(Client &client);
	void switch_poll(int client_fd, uint32_t events);


	bool allowed_in_path(const std::string &file_path, Client &client);

	void add_client_fd_to_epoll(int client_fd);

	bool check_timeout(Client *client, std::time_t timeout);
	int  get_pipefd_from_clientfd(int client_fd);

};
