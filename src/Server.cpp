/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 14:55:56 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/27 12:27:34 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/ConfigData.hpp"
#include "../includes/HttpRequest.hpp"

void Server::acceptClient(int serverFd)
{
    int client_fd = accept(serverFd, NULL, NULL);
    if (client_fd < 0)
        return;

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    _fds.push_back(pfd);

    Client client;
    client.fd = client_fd;
    client.buffer = "";
    client.config = _socketToConfig[serverFd]; // <--- NUEVO: Le decimos qué servidor usó para conectarse

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

/*
    handleClient:
        1. Calculamos dónde empiezan realmente los datos del fichero (Body)
            - Sumando esos 4 bytes invisibles del salto de línea de HTTP.
        2. Por defecto, asumimos que no hay cuerpo (0 bytes).
            - Buscamos si existe la etiqueta de longitud.
            - Solo la damos por válida si la encuentra ANTES de terminar los headers.
            - Extraemos el número "recortando" el texto. 
            - Sumamos 16 porque la palabra "Content-Length: " tiene 16 letras exactas.
            - atol() convierte "5000" (texto) a 5000 (número grande).
        3. Comprobamos la longitud real de lo que tenemos vs lo que debería ser

        4. Enviamos la respuesta al cliente
        5. Cerramos al cliente después de responder (comportamiento HTTP básico)
        
*/
void Server::handleClient(int clientFd)
{
    char buffer[1024];
    int bytes = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytes <= 0) 
    {
        removeClient(clientFd);
        return;
    }
    _clients[clientFd].buffer.append(buffer, bytes);
    std::string& client_buffer = _clients[clientFd].buffer;
   size_t pos_headers_end = client_buffer.find("\r\n\r\n");
    if (pos_headers_end != std::string::npos) 
    {
        size_t total_header_bytes = pos_headers_end + 4;
        long content_length = 0; 
        size_t pos_cl = client_buffer.find("Content-Length: ");
        if (pos_cl != std::string::npos && pos_cl < pos_headers_end) 
        {
            
            size_t pos_end_line = client_buffer.find("\r\n", pos_cl);
            std::string cl_str = client_buffer.substr(pos_cl + 16, pos_end_line - (pos_cl + 16));
            content_length = std::atol(cl_str.c_str());
        }
        if (client_buffer.length() < (total_header_bytes + content_length)) 
            return;
        std::cout << "[SERVER] Peticion completa recibida del fd " << clientFd << std::endl;
        HttpRequest req;
        req.parse(client_buffer);      
        std::string response = _httpHandler.handleRequest(req, *(_clients[clientFd].config));
        int sent = send(clientFd, response.c_str(), response.length(), 0);   
        if (sent > 0)
            std::cout << "[SERVER] Respuesta enviada al cliente " << clientFd << std::endl;
        removeClient(clientFd);
    }
}

void Server::removeClient(int fd)
{
    close(fd);
    _clients.erase(fd);
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