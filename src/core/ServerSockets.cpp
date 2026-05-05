/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSockets.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:10:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/03 20:10:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/*
    shutdown:
        1. close all active weekends
        2. clean internal lists
*/
void Server::shutdown()
{
    for (size_t i = 0; i < _fds.size(); i++)
        close(_fds[i].fd);
    _fds.clear();
    _clients.clear();
}

/*
    acceptClient:
        1. accepts incoming connection
        2. initialize the client and register it in the poll.
*/
void Server::acceptClient(int serverFd)
{
    int client_fd;
    pollfd pfd;
    Client client;

    client_fd = accept(serverFd, NULL, NULL);
    if (client_fd < 0)
        return;
    if (_clients.size() >= 900)
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
    client.config = _socketToConfigs[serverFd][0];
    _clients[client_fd] = client;
}

/*
    initSockets:
        1. create sockets, bind, and listen
        2. register each server in the poll
*/
void Server::initSockets()
{
    std::map<int, int> portToSocket;

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
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            std::cerr << "Error: setsockopt() failed for port "
                      << _configs[i].port << " :" << strerror(errno) << std::endl;
            close(server_fd);
            continue ;
        }
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

/*
    registredCgiFd:
        1. insert the pipe's fd into the poll
        2. associate the pipe with the customer
*/
void Server::registredCgiFd(int pipeFd, int clientFd)
{
    pollfd pdf;

    pdf.fd = pipeFd;
    pdf.events = POLLIN;
    pdf.revents = 0;
    _fds.push_back(pdf);

    _cgiPipeToClient[pipeFd] = clientFd;
}
