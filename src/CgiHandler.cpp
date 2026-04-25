#include "../includes/CgiHandler.hpp"
#include <sys/wait.h>

std::string CgiHandler::executeCgi(const std::string &scriptPath, const std::string &body)
{
	int pipe_in[2];
	int pipe_out[2];

	if(pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
		return "Error: Internal Server Error 500";
	pid_t pid = fork();
	if(pid < 0)
	{
		return "Error: Internal Server Error 500";
	}
	else if(pid == 0)
	{
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);
		char *args[] = {(char *)scriptPath.c_str(), NULL};
		execve(args[0], args, NULL);
		exit(1);
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
		write(pipe_in[1], body.c_str(), body.length());
		close(pipe_in[1]);
		waitpid(pid, NULL, 0);
		char buffer[4096];
		std::string result = "";
		ssize_t bytes_read;
		while((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytes_read] = '\0';
			result += buffer;
		}
		close(pipe_out[0]);
		return result;
	}
}
