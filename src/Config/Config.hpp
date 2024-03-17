#include <string>
#include <vector>
#include <cstdlib>

struct Virtual_Server_Config {
	unsigned int port;
};

class Config {
public:
	Config();
	~Config();

	Config(const std::string filename);
	std::vector<Virtual_Server_Config> get_virtual_servers();

	unsigned int port;

private:
	std::string filename;
	std::vector<Virtual_Server_Config> virtual_servers;
};
