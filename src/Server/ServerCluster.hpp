#pragma once

#include <map>
#include <vector>
#include "./Server.hpp"

class ServerCluster {
private:
	std::vector<Server> _servers;
	std::map<int, Server> _server_map;
	// To-Do delete entry if client closed
	std::map<int, Server> _client_fd_to_server_map;

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

		void	new_connection(int fd);
		void	handle_file_request(int client_fd, const std::string &file_path);
		void	handle_request(int fd);
		void	handle_write(int fd);
		void	handle_cgi_request(int client_fd, const std::string &cgi_script_path);
		void	handle_get_request(int client_fd, const std::string &requested_file_path);
		void	handle_delete_request(int client_fd, std::string full_path, std::string file_path);
		int		accept_new_connection(int server_fd);
		void	add_client_fd_to_epoll(int client_fd);
		Server 	*get_server_by_client_fd(int client_fd);
		void	handle_new_client_connection(int server_fd);





	FileUploader uploader;
};
