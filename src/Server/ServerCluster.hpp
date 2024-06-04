#pragma once

#include <map>
#include <sys/epoll.h>
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
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
#include "./HandleRequest.hpp"

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
	void switch_poll(int client_fd, uint32_t events);
	// Whats this?? To-DO
	FileUploader uploader;

	void handle_response(Client &client);
};