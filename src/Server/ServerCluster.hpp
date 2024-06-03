#pragma once

#include <map>
#include <sys/epoll.h>
#include "../Client/Client.hpp"
#include "./Server.hpp"

class ServerCluster {
private:
	std::map<int, Server> _server_map;
	std::map<int, Client> _client_map;

	int _epoll_fd;
	void logConfig(const Client &client);
	bool allowed_in_path(const std::string &file_path, Client &client);

public:
	ServerCluster(std::vector<Server> &servers);
	~ServerCluster();

	void close_client(int client_fd);
	void await_connections();
	void setup();
	void start();
	static void stop(int signal);

	void handle_new_client_connection(int server_fd);
	int accept_new_connection(int server_fd);

	void handle_request(Client &client);
	void handle_response(Client &client);

	void handle_cgi_request(const Client &client, const std::string &cgi_script_path);
	void handle_get_request(Client &client);
	void handle_post_request(Client &client);
	void handle_delete_request(Client &client);
	Client get_client_obj(epoll_event &event);
	void handle_delete_request(Client &client, std::string full_path);

	// 	void handle_file_request(const Client &client, const std::string &file_path);
	// 	void handle_write(const Client &client);
	// 	void handle_cgi_request(const Client &client, const std::string &cgi_script_path);
	// 	void handle_get_request(const Client &client, const std::string &requested_file_path);

	/* FileUploader uploader; */
	void handle_file_upload(Client &client);
	std::string extract_boundary(const std::string &headers);
	void switch_poll(int client_fd, uint32_t events);
};
