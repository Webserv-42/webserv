/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:34:52 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/27 15:23:37 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/ConfigData.hpp"

Client::Client() : fd(-1), serverFd(-1), buffer(""), bytesSend(0),
                   ContLength(0), state(READING_HEADERS), lastActivity(time(NULL)) {}
Client::Client(int fd) : fd(fd), buffer("") {}
Client::~Client() {}

void    Server::handleClient(int clientfd)
{
	Client& c = _clients[clientfd];
	
	switch (c.state)
	{
	case    READING_HEADERS:
	case    READING_BODY:
		ReadFromClient(c); // se llena con recv.
		if (c.state == PROCESSING)   // ← AÑADIR ESTO
        	ProcessRequest(c);
		break;
	case    PROCESSING:
		ProcessRequest(c); // se llena con el handleRequest.
		break;
	case    SENDING:
		sendResponse(c); // avanza con cada send, entre clientes.
		break;
	case    DONE:
		removeClient(clientfd); // destruye todo.
		break;
	default:
		break;
	}
}

void Server::ReadFromClient(Client& c)
{
	char	buffer[4096];
	int	n = recv(c.fd, buffer, sizeof(buffer), 0);

	if (n <= 0)
	{
		c.state = DONE; 
		return ;	
	}
    c.buffer.append(buffer, n);
	if (c.state == READING_HEADERS)
	{
		// Headers completos: READ_BODY
		// Parceo el Content - lenght de buffer 
		size_t posEnd = c.buffer.find("\r\n\r\n");
		if (posEnd != std::string::npos)
		{
    		size_t posCL = c.buffer.find("Content-Length: ");
    		if (posCL != std::string::npos && posCL < posEnd)
    		{
        		size_t endLine = c.buffer.find("\r\n", posCL);
        		std::string clStr = c.buffer.substr(posCL + 16, endLine - (posCL + 16));
        		c.ContLength = (size_t)std::atol(clStr.c_str());
				c.state = READING_BODY; 
			}
			else
        		c.state = PROCESSING;  
    	}
	}
	if (c.state == READING_BODY)
	{
		size_t headerEnd = c.buffer.find("\r\n\r\n") + 4;
		size_t bodyRecibed = c.buffer.size() - headerEnd;
		if (bodyRecibed >= c.ContLength)
			c.state = PROCESSING; // Si no esta retorna a poll para esperar mas datos.
	}
}

void Server::ProcessRequest(Client& c)
{
	ServerConfig *config = _socketToConfig[c.serverFd];
	
	HttpRequest req;
    req.parse(c.buffer);
	c.response = _httpHandler.handleRequest(req, *config);
	c.bytesSend = 0;
	c.state = SENDING;
	
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == c.fd)
		{
			_fds[i].events = POLLIN | POLLOUT;
			break ;
		}
		
	}
}

void Server::sendResponse(Client& c)
{
    const char* data = c.response.c_str() + c.bytesSend;
    size_t remaining = c.response.size() - c.bytesSend;
    int n = send(c.fd, data, remaining, 0);

    if (n < 0)
    {
        c.state = DONE;
        return;
    }
    c.bytesSend += n;
    if (c.bytesSend >= c.response.size())
    {
        // quitar POLLOUT antes de cerrar
        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].fd == c.fd)
            {
                _fds[i].events = POLLIN; // solo POLLIN, quitas POLLOUT
                break;
            }
        }
        c.state = DONE;
    }
}

void Server::removeClient(int fd)
{
    // cerrar socket
    close(fd);
    // borrar del mapa de clientes
    _clients.erase(fd);
    // eliminar de la lista de poll
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
    std::cout << "[CORE] Cliente eliminado: " << fd << std::endl;
}

