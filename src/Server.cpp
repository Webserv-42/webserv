/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 14:55:56 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/08 17:02:04 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/ConfigData.hpp"

void Server::acceptClient(int serverFd)
{
    int client_fd = accept(serverFd, NULL, NULL);
    if (client_fd < 0)
        return;

    // non-blocking
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    _fds.push_back(pfd);

    Client client;
    client.fd = client_fd;
    client.buffer = "";

    _clients[client_fd] = client;

    std::cout << "[CORE] Nuevo cliente conectado: " << client_fd << std::endl;
}

void Server::run()
{
    std::cout << "[CORE] Iniciando event loop...\n";

    while (true) {
        if (poll(&_fds[0], _fds.size(), -1) < 0) {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < _fds.size(); i++) {
            if (_fds[i].revents & POLLIN) {

                // 🟢 Nuevo cliente
                if (_socketToConfig.count(_fds[i].fd)) {
                    acceptClient(_fds[i].fd);
                }
                // 🔵 Cliente existente
                else {
                    handleClient(_fds[i].fd);
                }
            }
        }
    }
}

void Server::initSockets()
{
    for (size_t i = 0; i < _configs.size(); i++) {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            perror("socket");
            continue;
        }

        // reutilizar puerto
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // non-blocking
        fcntl(server_fd, F_SETFL, O_NONBLOCK);

        sockaddr_in addr;
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

        pollfd pfd;
        pfd.fd = server_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        _fds.push_back(pfd);
        _socketToConfig[server_fd] = &_configs[i];

        std::cout << "[CORE] Escuchando en puerto " << _configs[i].port << std::endl;
    }
}

void Server::handleClient(int clientFd)
{
    char buffer[1024];
    int bytes = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytes <= 0) {
        removeClient(clientFd);
        return;
    }

    _clients[clientFd].buffer.append(buffer, bytes);

    std::cout << _clients[clientFd].buffer << std::endl;
}

void Server::removeClient(int fd)
{
    // cerrar socket
    close(fd);

    // borrar del mapa de clientes
    _clients.erase(fd);

    // eliminar de la lista de poll
    for (size_t i = 0; i < _fds.size(); i++) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    std::cout << "[CORE] Cliente eliminado: " << fd << std::endl;
}

void Server::init(const std::vector<ServerConfig>& configs)
{
    _configs = configs;
}