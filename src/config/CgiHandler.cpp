/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 11:52:35 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/30 11:33:10 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <sys/wait.h>

CgiHandler::CgiHandler() 
{

}
CgiHandler::~CgiHandler() 
{
	
}

/*
    executeCgi:
        1. Crea las tuberías para la comunicación.
        2. Lanza el fork().
        3. Delega la lógica en executeChild o readParent según el proceso.
*/
std::string CgiHandler::executeCgi(const std::string &scriptPath, const HttpRequest &req)
{
	if (pipe(_pipeIn) < 0 || pipe(_pipeOut) < 0)
		return "Error: Internal Server Error 500";
		
	pid_t pid = fork();
	
	if (pid < 0)
		return "Error: Internal Server Error 500";
	else if (pid == 0) 
		executeChild(scriptPath, req);
		
	return (readParent(pid, req));
}


/*
    executeChild:
		- Prepara las tuberías, crea el entorno y llama a execve.
*/
void CgiHandler::executeChild(const std::string &scriptPath, const HttpRequest &req)
{
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	dup2(_pipeIn[0], STDIN_FILENO);
	dup2(_pipeOut[1], STDOUT_FILENO);
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	char **envp = createEnv(scriptPath, req);
	char *args[] = {(char *)scriptPath.c_str(), NULL};
	
	execve(args[0], args, envp);
	
	freeEnv(envp);
	_exit(1);
}

/*
    readParent:
    	- Escribe el body al CGI, espera a que termine y lee su respuesta.
*/
std::string CgiHandler::readParent(pid_t pid, const HttpRequest &req)
{
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	
	std::string body = req.getBody();
	write(_pipeIn[1], body.c_str(), body.length());
	close(_pipeIn[1]);
	
	time_t start = time(NULL);
    int status;
    bool timeout = false;
	while (waitpid(pid, &status, WNOHANG) == 0) 
    {
        if (time(NULL) - start >= 5)
        { 
            kill(pid, SIGKILL);
            timeout = true;
            break;
        }
        usleep(50000);
    }
	
	if (timeout) 
    {
        close(_pipeOut[0]);
        return ("HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 48\r\n\r\n<h1>504 Gateway Timeout</h1><p>CGI colgado</p>");
    }
	
	char buffer[4096];
	std::string result = "";
	ssize_t bytes_read;
	
	while ((bytes_read = read(_pipeOut[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytes_read] = '\0';
		result += buffer;
	}
	close(_pipeOut[0]);
	return (result);
}

/*
    createEnv:
    	- Crea el arreglo de char* necesario para pasar el entorno al CGI.
*/
char** CgiHandler::createEnv(const std::string &scriptPath, const HttpRequest &req)
{
	std::vector<std::string> envStrings;
	envStrings.push_back("REQUEST_METHOD=" + req.getMethod());
	envStrings.push_back("REQUEST_URI=" + req.getUri());
	envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envStrings.push_back("SCRIPT_FILENAME=" + scriptPath);
	
	std::stringstream ss;
	ss << req.getBody().length();
	envStrings.push_back("CONTENT_LENGTH=" + ss.str());
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
    	- Libera la memoria dinámica reservada para el entorno.
*/
void CgiHandler::freeEnv(char** envp)
{
	if (!envp) 
		return ;
		
	for (int i = 0; envp[i] != NULL; ++i) 
		delete[] envp[i];
		
	delete[] envp;
}