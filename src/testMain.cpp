#include <iostream>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "./Utils/utils.hpp"
#include "./Server/ServerCluster.hpp"
#include "./Server/Server.hpp"

int main () {
	populateContentTypes();
    std::vector<Server> servers;

    servers.push_back(Server(4222, "localhost"));
    servers.push_back(Server(8081, "0.0.0.0"));

    ServerCluster   cluster(servers);

    cluster.setupCluster();
    cluster.start();
    
    return 0;
}
