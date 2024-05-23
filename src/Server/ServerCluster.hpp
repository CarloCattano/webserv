#pragma once

#include "./Server.hpp"
#include <map>

class ServerCluster {
  private:
	std::vector<Server> _servers;
	std::map<int, Server> _server_map;
	int _epoll_fd;

	ServerCluster();

  public:
	ServerCluster(std::vector<Server> servers);
	~ServerCluster();

	void setupCluster();
	void await_connections();
	void setup();
	void start();
	static void stop(int signal);

	void new_connection(int fd);
	void handle_file_request(int client_fd, const std::string &file_path);
	void handle_request(int fd);
	void handle_write(int fd);
	void handle_cgi_request(int client_fd, const std::string &cgi_script_path);
	void handle_static_request(int client_fd, const std::string &requested_file_path,
							   const char *buffer);
	void handle_delete(int client_fd, std::string full_path, std::string file_path);

	FileUploader uploader;
};
