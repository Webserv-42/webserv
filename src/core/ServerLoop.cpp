/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLoop.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:10:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/03 20:10:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/*
    checkTimeouts:
        1. review the uptime of each client
        2. remove those that exceed the timeout
*/
void Server::checkTimeouts()
{
    time_t now = time(NULL);
    std::vector<int> toRemove;

    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        if (now - it->second.lastActivity > 30)
            toRemove.push_back(it->first);
        ++it;
    }
    for (size_t i = 0; i < toRemove.size(); i++)
        removeClient(toRemove[i]);
}

/*
    run:
        1. execute the main loop with poll()
        2. dispatches reading/writing events
*/
void Server::run()
{
    signal(SIGINT, Server::handleSigint);

    while (!_stop)
    {
        if (poll(&_fds[0], _fds.size(), 5000) < 0)
        {
            if (_stop)
                break;
            perror("poll");
            break;
        }

        checkTimeouts();

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & (POLLIN | POLLHUP | POLLERR))
            {
                if (_socketToConfigs.count(_fds[i].fd))
                    acceptClient(_fds[i].fd);
                else if (_cgiPipeToClient.count(_fds[i].fd))
                    handleCgiResponse(_fds[i].fd);
                else if (_cgiWritePipeToClient.count(_fds[i].fd))
                    handleCgiWrite(_fds[i].fd);
                else
                    handleClient(_fds[i].fd);
            }
            if (_fds[i].revents & POLLOUT)
            {
                if (_cgiWritePipeToClient.count(_fds[i].fd))
                    handleCgiWrite(_fds[i].fd);
                else
                    handleClient(_fds[i].fd);
            }
        }
    }
    shutdown();
}

/*
    handleCgiResponse:
        1. read the CGI pipe and accumulate response
        2. at the end, assemble headers and send to shipping
*/
void Server::handleCgiResponse(int pipeFd)
{
    int clientFd = _cgiPipeToClient[pipeFd];
    Client& c = _clients[clientFd];
    char buffer[4096];
    int n = read(pipeFd, buffer, sizeof(buffer));

    if (n <= 0)
    {
        close(pipeFd);
        removePollFd(pipeFd);
        _cgiPipeToClient.erase(pipeFd);
        setPollEvents(clientFd, POLLIN | POLLOUT);
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
