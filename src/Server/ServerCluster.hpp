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

/** @brief
 *  This class is responsible for handling the server cluster.
 *  It contains the server map, client map, pipeFd_clientFd_map, cgi_response_map, epoll_fd and the methods to handle
 *  the server cluster.
 *  @param _server_map: map of server objects where the servers instantiated are stored
 *  @param _client_map: map of clients with a file descriptor as the key and the client object as the value
 *  @param _pipeFd_clientFd_map: map of pipe file descriptors and client file descriptors
 *  @param _cgi_response_map: map of cgi responses with the client file descriptor as the key and the response as the
 * value
 *  @param _epoll_fd: epoll file descriptor
 */

class ServerCluster {
private:
	std::map<int, Server> _server_map;
	std::map<int, Client *> _client_map;
	std::map<int, time_t> _client_start_time_map;
	std::map<int, int> _pipeFd_clientFd_map;
	std::map<int, std::string> _cgi_response_map;
	int _epoll_fd;

	void handle_pipe_event(int pipe_fd);

public:
	ServerCluster(std::vector<Server> &servers);
	~ServerCluster();
	void start();

	/** @brief
	 *      This method is responsible for awaiting connections from the clients.
	 *      Using epoll to handle multiple connections, events that can be read, written or closed,
	 *      and changed at any time.
	 */
	void await_connections();

	void close_client(int client_fd);
	static void stop(int signal);

	/** @brief
	 *  This method is responsible for handling the new client connection.
	 *  @param server_fd: file descriptor of the server
	 */
	void handle_new_client_connection(int server_fd);

	/** @brief
	 *  This method is responsible for accepting the new connection.
	 *  @param server_fd: file descriptor of the server
	 *  @return int: file descriptor of the clientc
	 */
	int accept_new_connection(int server_fd);

	/** @brief
	 *  This method is responsible for handling the request from the client.
	 *  @param client: reference to the client object
	 */
	void handle_request(Client *client);

	/** @brief
	 *  This method is responsible for handling the response to the client.
	 *  @param client: reference to the client object
	 */
	void handle_response(Client *client);

	void handle_get_request(Client *client, Server *server);
	void handle_post_request(Client *client, Server *server);
	void handle_delete_request(Client *client, Server *server);
	void handle_redirection(Client *client, Server *server, HttpRedirection redirection);
	bool isPayloadTooLarge(Client *client);
	bool check_method_is_allowed(Client *client);

	/* FileUploader uploader; */
	void handle_file_upload(Client *client);
	std::string extract_boundary(Client *client);

	/** @brief A switch to modify the poll events
	 * to EPOLLIN or EPOLLOUT
	 * @param client_fd: file descriptor of the client
	 * @param events: events to be modified
	 **/
	void switch_poll(int client_fd, uint32_t events);

	bool allowed_in_path(const std::string &file_path, Client *client);

	void add_client_fd_to_epoll(int client_fd);

	bool check_timeout(Client *client, std::time_t timeout);
	void check_clients_timeout(std::map<int, Client *> &client_map, std::map<int, std::time_t> &client_start_time_map);

	/** @brief
	 * This method is responsible for getting the pipe file descriptor from the client file descriptor.
	 * @param client_fd: file descriptor of the client
	 * @return int: file descriptor of the pipe
	 */
	int get_pipefd_from_clientfd(int client_fd);
};
