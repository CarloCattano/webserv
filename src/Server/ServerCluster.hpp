#pragma once

#include "./Server.hpp"
#include <map>
#include "../Client/Client.hpp"
#include <sys/epoll.h>

class ServerCluster {
  private:
	std::map<int, Server>	_server_map;
	std::map<int, Client>	_client_map;

	int _epoll_fd;

	ServerCluster();

  public:
	ServerCluster(std::vector<Server> servers);
	~ServerCluster();

	void			await_connections();
	void			setup();
	void			start();
	static void		stop(int signal);

	void			handle_new_client_connection(int server_fd);
	int				accept_new_connection(int server_fd);
	
	void			handle_request(Client &client);
	void			handle_response(Client &client);
	
	void			handle_cgi_request(const Client &client, const std::string &cgi_script_path);
	void			handle_get_request(const Client &client, const std::string &requested_file_path);
	// void			handle_delete_request(const Client &client, std::string full_path, std::string file_path);
	Client			get_client_obj(epoll_event &event);

	FileUploader uploader;
	void switch_poll(int client_fd, uint32_t events);
};
