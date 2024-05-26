#include "./Config/Config.hpp"
#include "./Server/ServerCluster.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	std::string config_file;

	try
	{
		if (argc == 1) {
			config_file = "server.conf";
			std::cout << "Using default configuration file: server.conf" << std::endl;
		} else if (argc == 2)
			config_file = argv[1];
		else
			throw std::runtime_error("Usage: ./webserv [config_file]");

		Config config(config_file);
		ServerCluster cluster(config.get_servers());

		cluster.start();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
