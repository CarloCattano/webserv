#pragma once

#include <map>
#include <vector>
#include "./Server.hpp"

class ServerCluster {
	private:
		std::vector<Server>		_servers;
		std::map<int, Server>	_server_map;
		int						_epoll_fd;
		
		ServerCluster();

	public:
		ServerCluster(std::vector<Server> servers);

		void		setupCluster();
		void		await_connections();
		void 		setup();
		void 		start();
		static void	stop(int signal);

		void handle_file_request(int client_fd, const std::string &file_path);
		void handle_request(int fd);
		void handle_write(int fd);
		void handle_cgi_request(int client_fd, const std::string &cgi_script_path);
		void handle_static_request(int client_fd,
								const std::string &requested_file_path,
								const char *buffer);
		void handle_delete(int client_fd, std::string full_path, std::string file_path);


		FileUploader uploader;
};
