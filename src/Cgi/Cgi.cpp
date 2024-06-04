#include "./Cgi.hpp"
#include "../Utils/utils.hpp"

#define READ_END 0
#define WRITE_END 1

void handle_response(Client &client) {
	Response response = client.getResponse();

	std::string response_string = client.responseToString();
	client.setResponseSize(response_string.size());
	client.setSentBytes(client.getSentBytes() +
						send(client.getFd(), response_string.c_str(), 4096, 0));

	if (client.getSentBytes() >= response_string.size()) {
	}
}

void handle_cgi_request(Client &client, char *cgi_script_path) {
	(void)client;

	pid_t	pid;

	pid = fork();
	if (pid == -1)
		throw (std::runtime_error("Fork failed."));
	if (pid == 0)
	{
		std::string output = execute_cgi_script(cgi_script_path);
		std::cout << output << std::endl;
		client.setResponseBody(output);
		client.setResponseStatusCode(200);
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Content-Length", intToString(output.size()));
		client.addResponseHeader("Connection", "keep-alive");

		
		handle_response(client);



		exit(EXIT_SUCCESS);
	}


		// int status;
		// if (waitpid(pid, &status, 0) == -1)
		// 	throw std::runtime_error("Waitpid 2 failed.");

		// if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		// 	throw std::runtime_error("Child process failed.");
}

std::string execute_cgi_script(char *cgi_script_path) {
	int 	pipe_fd[2];
	pid_t 	pid;

	if (pipe(pipe_fd) == -1)
		throw (std::runtime_error("Pipe creation failed."));

	char *av[] = {cgi_script_path, NULL};

	pid = fork();
	if (pid == -1)
		throw (std::runtime_error("Fork failed."));

	if (pid == 0)
	{
		close(pipe_fd[READ_END]);
        dup2(pipe_fd[WRITE_END], STDOUT_FILENO);
        close(pipe_fd[WRITE_END]);
        execve(cgi_script_path, av, NULL);
		exit(EXIT_FAILURE);
    }
	else
	{
		char 	buffer[1024];
		std::string	result;
        ssize_t	bytes_read = 1;

		close(pipe_fd[WRITE_END]);

        while (bytes_read > 0) {
			bytes_read = read(pipe_fd[READ_END], buffer, sizeof(buffer));
			if (bytes_read == -1)
				throw (std::runtime_error("Error reading from pipe."));
			buffer[bytes_read] = '\0';
			result += buffer;
        }
        close(pipe_fd[WRITE_END]);

		return (result);
	}
}

		// wait(NULL);

		// int status;
		// if (waitpid(pid, &status, 0) == -1)
		// 	throw std::runtime_error("Waitpid 1 failed.");

		// if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		// 	throw std::runtime_error("Child process failed.");