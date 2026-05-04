/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 14:55:56 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/03 16:50:17 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ConfigData.hpp"
#include "HttpRequest.hpp"

volatile sig_atomic_t Server::_stop = 0;

void Server::handleSigint(int)
{
	_stop = 1;
}

void Server::shutdown()
{
    for (size_t i = 0; i < _fds.size(); i++)
        close(_fds[i].fd);
    _fds.clear();
    _clients.clear();
}

void Server::acceptClient(int serverFd)
{
	int client_fd;
	pollfd pfd;
	Client client;

    client_fd = accept(serverFd, NULL, NULL);
    if (client_fd < 0)
        return;
    if (_clients.size() >= 900) // limita el numero de clientes del servidor
    {
        close(client_fd);
        return ;
    }
    
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_fds.push_back(pfd);

	client.fd = client_fd;
	client.serverFd = serverFd;
    client.config = _socketToConfigs[serverFd][0]; // Usa el primero por defecto
	_clients[client_fd] = client;
}

void Server::checkTimeouts()
{
    time_t now = time(NULL);
    std::vector<int> toRemove;

    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        if (now - it->second.lastActivity > 30) // 30 segundos
        {
            toRemove.push_back(it->first);
        }
        ++it;
    }
    for (size_t i = 0; i < toRemove.size(); i++)
        removeClient(toRemove[i]);
}

void Server::run()
{
    signal(SIGINT, Server::handleSigint);

    while (!_stop)
    {
        if (poll(&_fds[0], _fds.size(), 5000) < 0)
        {
            if (_stop) break;
            perror("poll");
            break;
        }

        checkTimeouts();

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_socketToConfigs.count(_fds[i].fd))
                    acceptClient(_fds[i].fd);
		        else if (_cgiPipeToClient.count(_fds[i].fd))
			        handleCgiResponse(_fds[i].fd);
                else
                    handleClient(_fds[i].fd);
            }
            if (_fds[i].revents & POLLOUT)
                handleClient(_fds[i].fd);
        }
    }
    shutdown();
}

void	Server::handleCgiResponse(int pipeFd)
{
	int	clientFd = _cgiPipeToClient[pipeFd];
	Client&	c = _clients[clientFd];
	char	buffer[4096];
	int	n;
	n = read(pipeFd, buffer, sizeof(buffer));
	
	if (n <= 0)
	{
		close(pipeFd);
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].fd == pipeFd)
			{
				_fds.erase(_fds.begin() + i);
				break ;
			}
		}
		_cgiPipeToClient.erase(pipeFd);
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].fd == clientFd)
			{
				_fds[i].events = POLLIN | POLLOUT;
				break ;
			}
		}
		size_t headerEnd = c.response.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			size_t bodyLen = c.response.length() - (headerEnd + 4);
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK\r\nContent-Length: " << bodyLen << "\r\n";
			c.response = ss.str() + c.response;
		}
		else
		{
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK\r\nContent-Length: " << c.response.length() << "\r\n\r\n";
			c.response = ss.str() + c.response;
		}
		c.bytesSend = 0;
		c.state = SENDING;
		return ;
	}
	c.response.append(buffer, n);
	c.lastActivity = time(NULL);
}

void Server::initSockets()
{
    std::map<int, int> portToSocket; // Para no duplicar puertos

    for (size_t i = 0; i < _configs.size(); i++)
    {
        int port = _configs[i].port;
        if (portToSocket.find(port) != portToSocket.end())
        {
            int server_fd = portToSocket[port];
            _socketToConfigs[server_fd].push_back(&_configs[i]);
            continue;
        }

        int server_fd;
        sockaddr_in addr;
        pollfd pfd;
    
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
		std::cerr << "Error: Socket() open server_fd. " << strerror(errno) << std::endl;
		continue;
        }
        // reutilizar puerto
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error: setsockopt() failed for port " << 
			_configs[i].port << " :" << strerror(errno) << std::endl;
		close (server_fd);
		continue ;
	}
        // non-blocking
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_configs[i].port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            close(server_fd);
            continue;
        }
        if (listen(server_fd, SOMAXCONN) < 0)
        {
            perror("listen");
            close(server_fd);
            continue;
        }
        pfd.fd = server_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _fds.push_back(pfd);
        _socketToConfigs[server_fd].push_back(&_configs[i]);
        portToSocket[port] = server_fd;
        std::cout << "Listen on port " << _configs[i].port << std::endl;
    }
}

void    Server::registredCgiFd(int  pipeFd, int clientFd)
{
    pollfd	pdf;

    pdf.fd = pipeFd;
    pdf.events = POLLIN;
    pdf.revents = 0;
    _fds.push_back(pdf);

    _cgiPipeToClient[pipeFd] = clientFd;
}

void Server::init(const std::vector<ServerConfig>& configs)
{
    _configs = configs;
}


