/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/22 13:26:15 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

/*
    Aquí recibimos la cadena de una peticion HTTP, parsearla, aplicar las reglas
    de ruteo (ServerConfig), ejecutar CGI si es necesario mediante fork/execve
    y construir el string de respuesta HTTP final.
    
    Aquí trabaja " "
*/
/*
    class HttpHandler:
        - Aqui tenemos que implementar el parseo del rawRequest, verificar
            métodos, comprobar archivos estáticos o ejecutar CGI segun la
            serverConf.
*/
class HttpHandler
{
    public:
        HttpHandler() {}
        ~HttpHandler() {}
        
        std::string handleRequest(const std::string& rawRequest, const ServerConfig& serverConf) {
            (void)rawRequest;
            (void)serverConf;
            
            std::cout << "[DEV 3] Procesando peticion HTTP y generando respuesta..." << std::endl;
            

            return "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello Webserv";
        }
};

#endif