#pragma once

#include <string>
#include "../Client/Client.hpp"

class Cgi {
public:
	Cgi();
	~Cgi();

	void handle_cgi_request(Client &client, const std::string &cgi_script_path, std::map<int, int> &_pipeFd_clientFd_map, int epoll_fd);

private:
	std::string _cgi;

	Cgi(const Cgi &src);
	Cgi &operator=(const Cgi &src);
};
