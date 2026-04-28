/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 14:55:56 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/28 10:42:29 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/ConfigData.hpp"
#include "../includes/HttpRequest.hpp"

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
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_fds.push_back(pfd);

	client.fd = client_fd;
	client.serverFd = serverFd;
    client.config = _socketToConfig[serverFd]; // <--- NUEVO: Le decimos qué servidor usó para conectarse
	_clients[client_fd] = client;
	std::cout << "[CORE] Nuevo cliente conectado: " << client_fd - 3 << std::endl;
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
            std::cout << "[CORE] Timeout cliente: " << it->first << std::endl;
            toRemove.push_back(it->first);
        }
        ++it;
    }
    for (size_t i = 0; i < toRemove.size(); i++)
        removeClient(toRemove[i]);
}

// void Server::run()
// {
//     std::cout << "[CORE] Iniciando event loop...\n";

//     while (true)
//     {
//         if (poll(&_fds[0], _fds.size(), -1) < 0)
//         {
//             perror("poll");
//             break;
//         }
//         for (size_t i = 0; i < _fds.size(); i++)
//         {
//             if (_fds[i].revents & POLLIN)
//             {
//                 // Nuevo cliente
//                 if (_socketToConfig.count(_fds[i].fd))
//                     acceptClient(_fds[i].fd);
//                 // Cliente existente
//                 else
//                     handleClient(_fds[i].fd);
//             }
// 	    if (_fds[i].revents & POLLOUT)
// 	    	handleClient(_fds[i].fd); // llega al estado SENDING.
//         }
//     }
// }

void Server::run()
{
    signal(SIGINT, Server::handleSigint);
    std::cout << "[CORE] Iniciando event loop...\n";

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
                if (_socketToConfig.count(_fds[i].fd))
                    acceptClient(_fds[i].fd);
                else
                    handleClient(_fds[i].fd);
            }
            if (_fds[i].revents & POLLOUT)
                handleClient(_fds[i].fd);
        }
    }
    shutdown();
}

void Server::initSockets()
{
    for (size_t i = 0; i < _configs.size(); i++)
    {
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
        _socketToConfig[server_fd] = &_configs[i];
        std::cout << "[CORE] listen on port " << _configs[i].port << std::endl;
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


void Server::init(const std::vector<ServerConfig>& configs)
{
    _configs = configs;
}


