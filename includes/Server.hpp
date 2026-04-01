/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:09 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/01 13:34:27 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigData.hpp"
#include "HttpHandler.hpp"
#include <vector>
#include <iostream>

/*
    Gestion de red, crear sockets, hacer bind/ listen,
    manejar el event pool con poll().
    Delega toda la logica HTPP a HttpHandler.
    Aqui trabaja " "
*/

/*
    class Server:
        - Tiene que contener la logica de socket(), setsockopt(), bind(), 
            listen(), fcntl()
        - Un bucle infinito donde:
            Se implementar el loop de poll()
            Cuando poll() indique que hay datos listos para leer:
                1. LLamar a recv() para obtener el texto
                2. Pasar el texto al HtppHandler
        - Delegar proceso al HtppHandler pasandole la configuracion del servidor correspondiente
        - Cuando poll() indique que podemos escribir (POLLOUT), enviar la respuesta con send()
*/
class Server {
private:
    std::vector<ServerConfig> _configs;
    HttpHandler _httpHandler;

public:
    Server() {}
    ~Server() {}

    void init(const std::vector<ServerConfig>& configs) {
        _configs = configs;
        std::cout << "[DEV 1] Inicializando sockets para " << _configs.size() << " servidores..." << std::endl;
    }
    
    void run() {
        std::cout << "[DEV 1] Iniciando loop de eventos (poll)... Presiona Ctrl+C para salir." << std::endl;
        std::string mockRequest = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
        
        std::string response = _httpHandler.handleRequest(mockRequest, _configs[0]);
        
        std::cout << "[DEV 1] Respuesta recibida del módulo HTTP. Enviando al cliente..." << std::endl;
    }
};

#endif