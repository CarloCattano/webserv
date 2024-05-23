#include "./Config/Config.hpp"
#include "./Server/ServerCluster.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

	std::vector<Server> servers;
	std::string config_file = "";

	if (argv[1] == NULL || argc != 2) {
		// use default config file as per the project requirement
		config_file = "server.conf";
		std::cout << "Using default configuration file: " << config_file << std::endl;
	} else {
		config_file = argv[1];

		if (config_file == "" || config_file.empty()) {
			std::cerr << "Invalid configuration file" << std::endl;
			return 1;
		}
	}

	Config config(config_file);

	servers = config.get_servers();
	ServerCluster cluster(servers);

	cluster.setupCluster();
	cluster.start();

	return 0;
}
