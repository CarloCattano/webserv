#include "./Cgi.hpp"
#define READ_END 0
#define WRITE_END 1

void handle_cgi_request(const Client &client, char *cgi_script_path) {
	(void)client;
	int 	pipefd[2];
	pid_t 	pid;

	if (pipe(pipefd) == -1)
		throw (std::runtime_error("Pipe creation failed."));

	char *av[] = {cgi_script_path, NULL};

	pid = fork();
	if (pid == -1)
		throw (std::runtime_error("Fork failed."));

	if (pid == 0)
	{
		close(pipefd[READ_END]);

		// Here we need to dup to the epoll fd

		// do i even need pipes ???
		
        dup2(client.getFd(), STDOUT_FILENO);

        close(pipefd[WRITE_END]);
        execve(cgi_script_path, av, NULL);
		exit(EXIT_FAILURE);
    }
	else
	{
		close(pipefd[READ_END]);
		close(pipefd[WRITE_END]);
        int status;
        if (waitpid(pid, &status, 0) == -1)
			throw (std::runtime_error("Waitpid failed."));
	}
}