/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/23 10:47:05 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"
#include "HttpRequest.hpp"

/*
    Aquí recibimos la cadena de una peticion HTTP, parsearla, aplicar las reglas
    de ruteo (ServerConfig), ejecutar CGI si es necesario mediante fork/execve
    y construir el string de respuesta HTTP final.
*/
/*
    class HttpHandler:
        - Aqui tenemos que implementar el parseo del rawRequest, verificar
            métodos, comprobar archivos estáticos o ejecutar CGI segun la
            serverConf.
*/

class HttpHandler
{
    private:
        std::string getMimeType(const std::string& filePath);
        const LocationConfig* matchLocation(const std::string& uri, const ServerConfig& serverConf);
        std::string buildErrorResponse(int statusCode);
        std::string generateDirectoryListing(const std::string& physicalPath, const std::string& currentUri);
    public:
        HttpHandler();
        ~HttpHandler();
        std::string handleRequest(HttpRequest& req, const ServerConfig& serverConf); 
};

#endif