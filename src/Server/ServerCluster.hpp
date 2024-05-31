#pragma once

#include "./Server.hpp"
#include <map>
#include "../Response/Response.hpp"

struct Client
{
	int			fd;
	Server		*server;
	Response	*response;
};


class ServerCluster {
  private:
	std::vector<Server>&		_servers;
	std::map<int, Server>		_server_map;
	// To-Do delete entry if client closed
	std::map<int, Server>		_client_fd_to_server_map;

	int _epoll_fd;

  public:
	ServerCluster(std::vector<Server>& servers);
	~ServerCluster();

	void			setupCluster();
	void			await_connections();
	void			setup();
	void			start();
	static void		stop(int signal);

	void			new_connection(int fd);
	void			handle_file_request(const Client &client, const std::string &file_path);
	void			handle_request(const Client &client);
	void			handle_write(const Client &client);
	void			handle_cgi_request(const Client &client, const std::string &cgi_script_path);
	void			handle_get_request(const Client &client, const std::string &requested_file_path);
	void			handle_delete_request(const Client &client, std::string full_path, std::string file_path);
	int				accept_new_connection(int server_fd);
	void			add_client_fd_to_epoll(int client_fd);
	void			handle_new_client_connection(int server_fd);
	Client			get_client_obj(const int &client_fd);

	FileUploader uploader;
	void switch_poll(int client_fd, uint32_t events);
};
