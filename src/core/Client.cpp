/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:34:52 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/03 16:51:43 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "ConfigData.hpp"

Client::Client() : fd(-1), serverFd(-1), buffer(""), bytesSend(0),
                   ContLength(0), state(READING_HEADERS), lastActivity(time(NULL)), keepAlive(false) 
{
	
}
Client::~Client() 
{
	
}

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
	case    CGI_WAITING:
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
// Modificado.
void Server::ReadFromClient(Client& c)
{
	char	buffer[4096];
	int	n = recv(c.fd, buffer, sizeof(buffer), 0);

	if (n <= 0)
	{
		c.state = DONE; 
		return ;	
	}
	c.lastActivity = time(NULL);
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
				if(c.ContLength > (size_t)c.config->clientMaxBodySize)
				{
					c.response = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\n\r\n";
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
					return;
				}
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
// Modificado para uso de las cookies
void Server::ProcessRequest(Client& c)
{
	std::string	cookie = "";
	size_t	pos = c.buffer.find("Cookie: ");
	if (pos != std::string::npos)
	{
		size_t end = c.buffer.find("\r\n", pos);
        cookie = c.buffer.substr(pos + 8, end - (pos + 8));
	}
	c.cookie = cookie;
	ServerConfig *config = _socketToConfig[c.serverFd];
	
	HttpRequest req;
    req.parse(c.buffer);
    
    // --- Comprobar Keep-Alive de forma segura ---
    c.keepAlive = false;
    std::map<std::string, std::string> headers = req.getHeaders();
    if (headers.find("Connection") != headers.end()) 
    {
        std::string connStr = headers["Connection"];
        for (size_t i = 0; i < connStr.length(); i++) {
            connStr[i] = std::tolower(connStr[i]);
        }
        if (connStr.find("keep-alive") != std::string::npos) {
            c.keepAlive = true;
        }
    }
    
	int cgiPipeFd = -1;
	c.response = _httpHandler.handleRequest(req, *config, &cgiPipeFd);
	c.bytesSend = 0;
	if (cgiPipeFd != -1)
	{
		registredCgiFd(cgiPipeFd, c.fd);
		c.state = CGI_WAITING;
	}
	else
	{
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
}
// Modificado
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
	if (n > 0)
		c.lastActivity = time(NULL);
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
		if (c.keepAlive) // Revisa si el cliente sigue vivo usando los headers (no el buffer crudo)
		{
			// resetear el cliente para la siguiente llamada.
			c.buffer.clear();
			c.response.clear();
			c.bytesSend = 0;
			c.ContLength = 0;
			c.keepAlive = false;
			c.state = READING_HEADERS; // Vuelve a esperar para leer.
		}
		else
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
}

