#pragma once

#include "../Config/Config.hpp"
#include "../Server/Server.hpp"
#include <map>
#include <vector>

class ServerCluster {
  public:
	std::vector<Server> servers;
	std::map<int, Server> serverMap;
	// TODO need client
	/* std::map<int, */

	ServerCluster(Config config);

	// store a map of
};
