#pragma once
#include "ServerCluster.hpp"
#include <string>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>

void handle_cgi_request(Client &client, char *cgi_script_path);
std::string execute_cgi_script(char *cgi_script_path);

// class Cgi {
// public:
// 	Cgi();
// 	~Cgi();
// 	std::string run(const std::string &script);

// private:
// 	std::string _cgi;

// 	Cgi(const Cgi &src);
// 	Cgi &operator=(const Cgi &src);
// };
