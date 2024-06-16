#pragma once

#include <string>
#include "../Client/Client.hpp"

class Cgi {
public:
	Cgi();
	~Cgi();

	/** @brief
	 *      This method is responsible for handling the cgi request.
	 *      It creates a pipe, forks the process, and executes the cgi script, while handling the request.
	 *
	 *  @param client: client object
	 *  @param cgi_script_path: path to the cgi script
	 *  @param _pipeFd_clientFd_map: map of pipe file descriptors and client file descriptors
	 *  @param epoll_fd: epoll file descriptor
	 */
	void handle_cgi_request(Client *client, const std::string &cgi_script_path,
							std::map<int, int> &_pipeFd_clientFd_map, int epoll_fd);

private:
	std::string _cgi;

	Cgi(const Cgi &src);
	Cgi &operator=(const Cgi &src);
};
