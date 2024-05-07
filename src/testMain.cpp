#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "./Server/Server.hpp"
#include "./Server/ServerCluster.hpp"
#include "./Utils/utils.hpp"

int main()
{
	populateContentTypes();
	std::vector<Server> servers;

	servers.push_back(Server(4222, "0.0.0.0"));
	servers.push_back(Server(8081, "127.0.0.69"));

	ServerCluster cluster(servers);

	cluster.setupCluster();
	cluster.start();

	return 0;
}
