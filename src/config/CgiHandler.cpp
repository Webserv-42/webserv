/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 11:52:35 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 19:26:49 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <cctype>
#include <sys/wait.h>

/*
	CgiHandler:
		1. initializes the CGI handler
*/
CgiHandler::CgiHandler() 
{

}
/*
	~CgiHandler:
		1. free up resources if necessary
*/
CgiHandler::~CgiHandler() 
{
	
}

/*
    executeCgi:
        1. create the channels for communication
        2. launch the fork()
        3. in the child process: delegate the logic to executeChild
        4. in the parent process: Write the request body
			and return the read function (non-blocking)
*/
int CgiHandler::executeCgi(const std::string &executablePath, const std::string &scriptPath,
	const HttpRequest &req, int *writeFdOut)
{
	if (pipe(_pipeIn) < 0 || pipe(_pipeOut) < 0)
		return (-1);	
	pid_t pid = fork();
	if (pid < 0)
		return (-1);
	else if (pid == 0) 
		executeChild(executablePath, scriptPath, req);
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	fcntl(_pipeIn[1], F_SETFL, O_NONBLOCK);
	fcntl(_pipeOut[0], F_SETFL, O_NONBLOCK);
	if (writeFdOut != NULL)
		*writeFdOut = _pipeIn[1];
	return (_pipeOut[0]);
}



/*
    executeChild:
		- prepare the pipes, create the environment, and call execve.
*/
void CgiHandler::executeChild(const std::string &executablePath, const std::string &scriptPath, const HttpRequest &req)
{
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	dup2(_pipeIn[0], STDIN_FILENO);
	dup2(_pipeOut[1], STDOUT_FILENO);
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	std::string scriptFile = scriptPath;
	size_t slash = scriptFile.find_last_of('/');
	if (slash != std::string::npos)
	{
		std::string scriptDir = scriptFile.substr(0, slash);
		scriptFile = scriptFile.substr(slash + 1);
		if (!scriptDir.empty())
			chdir(scriptDir.c_str());
	}
	char **envp = createEnv(scriptPath, req);
	char *args[] = 
	{
		(char *)executablePath.c_str(), 
		(char *)scriptFile.c_str(), 
		NULL
	};
	execve(args[0], args, envp);
	freeEnv(envp);
	_exit(1);
}

/*
    createEnv:
    	- create the necessary char* array to pass the environment to the CGI
*/
char** CgiHandler::createEnv(const std::string &scriptPath, const HttpRequest &req)
{
	std::vector<std::string> envStrings;
	std::map<std::string, std::string> headers = req.getHeaders();
	std::string uri = req.getUri();
	std::string path = uri;
	std::string query = "";
	size_t queryPos = uri.find('?');
	if (queryPos != std::string::npos)
	{
		path = uri.substr(0, queryPos);
		query = uri.substr(queryPos + 1);
	}

	envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
	envStrings.push_back("REQUEST_URI=" + uri);
	envStrings.push_back("QUERY_STRING=" + query);
	envStrings.push_back("SCRIPT_NAME=" + path);
	envStrings.push_back("PATH_INFO=");
	envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);

	std::string contentType = "";
	if (headers.find("Content-Type") != headers.end())
		contentType = headers["Content-Type"];
	envStrings.push_back("CONTENT_TYPE=" + contentType);

	std::stringstream ss;
	ss << req.getBody().length();
	envStrings.push_back("CONTENT_LENGTH=" + ss.str());

	if (headers.find("Host") != headers.end())
	{
		std::string host = headers["Host"];
		std::string port = "";
		size_t colon = host.find(':');
		if (colon != std::string::npos)
		{
			port = host.substr(colon + 1);
			host = host.substr(0, colon);
		}
		if (!host.empty())
			envStrings.push_back("SERVER_NAME=" + host);
		if (!port.empty())
			envStrings.push_back("SERVER_PORT=" + port);
	}

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "Content-Type" || it->first == "Content-Length")
			continue;
		std::string envKey = "HTTP_";
		for (size_t i = 0; i < it->first.size(); ++i)
		{
			unsigned char ch = static_cast<unsigned char>(it->first[i]);
			if (std::isalnum(ch))
				envKey += static_cast<char>(std::toupper(ch));
			else
				envKey += '_';
		}
		envStrings.push_back(envKey + "=" + it->second);
	}
	char** envp = new char*[envStrings.size() + 1];
	for (size_t i = 0; i < envStrings.size(); ++i) 
	{
		envp[i] = new char[envStrings[i].length() + 1];
		std::strcpy(envp[i], envStrings[i].c_str());
	}
	envp[envStrings.size()] = NULL;
	
	return (envp);
}

/*
    freeEnv:
    	- free up the dynamic memory reserved for the environment
*/
void CgiHandler::freeEnv(char** envp)
{
	if (!envp) 
		return ;
		
	for (int i = 0; envp[i] != NULL; ++i) 
		delete[] envp[i];
		
	delete[] envp;
}