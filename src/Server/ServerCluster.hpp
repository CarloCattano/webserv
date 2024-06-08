#pragma once

#include <map>
#include <string>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../Client/Client.hpp"
#include "./Server.hpp"

/** @brief ServerCluster class
 *  This class is responsible for handling the server cluster.
 *  It contains the server map, client map, pipeFd_clientFd_map, cgi_response_map, epoll_fd and the methods to handle
 *  the server cluster.
 *  @param _server_map: map of server objects
 *  @param _client_map: map of client objects
 *  @param _pipeFd_clientFd_map: map of pipe file descriptors and client file descriptors
 *  @param _cgi_response_map: map of cgi responses
 *  @param _epoll_fd: epoll file descriptor
 */

class ServerCluster {
private:
	std::map<int, Server> _server_map;
	std::map<int, Client> _client_map;
	std::map<int, int> _pipeFd_clientFd_map;
	std::map<int, std::string> _cgi_response_map;
	int _epoll_fd;

	void handle_pipe_event(int pipe_fd);

public:
	ServerCluster(std::vector<Server> &servers);
	~ServerCluster();
	void start();

	/** @brief await_connections
	 *  This method is responsible for awaiting connections from the clients.
	 *  Using epoll to handle multiple connections, events that can be read, written or closed,
	 *  and changed at any time.
	 */
	void await_connections();

	void close_client(int client_fd);
	static void stop(int signal);

	/** @brief handle_new_client_connection
	 *  This method is responsible for handling the new client connection.
	 *  @param server_fd: file descriptor of the server
	 */
	void handle_new_client_connection(int server_fd);

	/** @brief accept_new_connection
	 *  This method is responsible for accepting the new connection.
	 *  @param server_fd: file descriptor of the server
	 *  @return int: file descriptor of the clientc
	 */
	int accept_new_connection(int server_fd);

	/** @brief handle_request
	 *  This method is responsible for handling the request from the client.
	 *  @param client: reference to the client object
	 */
	void handle_request(Client &client);

	/** @brief handle_response
	 *  This method is responsible for handling the response to the client.
	 *  @param client: reference to the client object
	 */
	void handle_response(Client &client);

	void handle_get_request(Client &client, Server *server);
	void handle_post_request(Client &client, Server *server);
	void handle_delete_request(Client &client, Server *server);
	Client get_client_obj(epoll_event &event);

	/* FileUploader uploader; */
	void handle_file_upload(Client &client);
	std::string extract_boundary(const std::string &headers);
	void switch_poll(int client_fd, uint32_t events);

	bool allowed_in_path(const std::string &file_path, Client &client);

	void add_client_fd_to_epoll(int client_fd);

	void check_timeout(Client &client, int timeout);

	/** @brief get_pipefd_from_clientfd
	 * This method is responsible for getting the pipe file descriptor from the client file descriptor.
	 * @param client_fd: file descriptor of the client
	 * @return int: file descriptor of the pipe
	 */
	int get_pipefd_from_clientfd(int client_fd);
};
