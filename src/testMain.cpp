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

    servers.push_back(Server(8080, inet_addr(std::string("127.0.0.1").data())));
    servers.push_back(Server(8081, inet_addr(std::string("127.0.0.2").data())));

    ServerCluster   cluster(servers);

    cluster.setupCluster();
    cluster.start();
    
    return 0;
}
