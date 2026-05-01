/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 11:52:35 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/01 18:05:05 by gafreire         ###   ########.fr       */
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
        1. Crea las tuberías para la comunicación
        2. Lanza el fork()
        3. En el proceso hijo: Delega la lógica en executeChild
        4. En el proceso padre: Escribe el body de la petición 
			y devuelve el FD de lectura (No bloqueante)
*/
int CgiHandler::executeCgi(const std::string &executablePath, const std::string &scriptPath, const HttpRequest &req)
{
	if (pipe(_pipeIn) < 0 || pipe(_pipeOut) < 0)
		return -1;	
	pid_t pid = fork();
	if (pid < 0)
		return -1;
	else if (pid == 0) 
		executeChild(executablePath, scriptPath, req);
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	std::string body = req.getBody();
	if (!body.empty())
		write(_pipeIn[1], body.c_str(), body.length());
	close(_pipeIn[1]);
	return _pipeOut[0];
}



/*
    executeChild:
		- Prepara las tuberías, crea el entorno y llama a execve
*/
void CgiHandler::executeChild(const std::string &executablePath, const std::string &scriptPath, const HttpRequest &req)
{
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	dup2(_pipeIn[0], STDIN_FILENO);
	dup2(_pipeOut[1], STDOUT_FILENO);
	close(_pipeIn[0]);
	close(_pipeOut[1]);
	char **envp = createEnv(scriptPath, req);
	char *args[] = 
	{
		(char *)executablePath.c_str(), 
		(char *)scriptPath.c_str(), 
		NULL
	};
	execve(args[0], args, envp);
	freeEnv(envp);
	_exit(1);
}

/*
    createEnv:
    	- Crea el arreglo de char* necesario para pasar el entorno al CGI
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
    	- Libera la memoria dinámica reservada para el entorno
*/
void CgiHandler::freeEnv(char** envp)
{
	if (!envp) 
		return ;
		
	for (int i = 0; envp[i] != NULL; ++i) 
		delete[] envp[i];
		
	delete[] envp;
}