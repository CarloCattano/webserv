#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "./Server/ServerCluster.hpp"
#include "./Utils/utils.hpp"

int main () {
    std::vector<Server> servers;

    servers.push_back(Server(4222, "127.0.0.1"));
    servers.push_back(Server(8081, "127.0.0.69"));

    ServerCluster   cluster(servers);
    cluster.start();
    
    return 0;
}
