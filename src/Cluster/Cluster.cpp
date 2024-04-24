#include "Cluster.hpp"
#include "../Server/Server.hpp"

#include <sys/wait.h>
#include <unistd.h>

Cluster::~Cluster() {}

Cluster::Cluster(const Config &config) {

	unsigned int size = config.get_virtual_servers().size();

	for (unsigned int i = 0; i < size; i++) {
		pid_t pid = fork();
		if (pid == 0) {
			Server server("123", config.get_virtual_servers()[i].port);
		} else if (pid < 0) {
			throw std::runtime_error("Fork failed");
		} else {
			// Parent process
		}
		while (waitpid(pid, NULL, WNOHANG) != -1) {
			continue;
		}
	}
}
