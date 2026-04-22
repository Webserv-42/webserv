/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:30:26 by gafreire         ###   ########.fr       */
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
	private:
		const LocationConfig *findLocation(const std::string &uri, const ServerConfig &serverConf);
		std::string getStaticFileContent(const std::string &uri, const LocationConfig &location);
		bool saveUploadedFile(const std::string &filename, const std::string &fileContent, const LocationConfig &location);
		bool deleteFile(const std::string &uri, const LocationConfig &location);
		std::string getErrorPageContent(int errorCode, const LocationConfig& location);
        std::string getMimeType(const std::string& filePath);
        const LocationConfig* matchLocation(const std::string& uri, const ServerConfig& serverConf);
        std::string buildErrorResponse(int statusCode);
    public:
        HttpHandler();
        ~HttpHandler();
        std::string handleRequest(HttpRequest& req, const ServerConfig& serverConf); 
};

#endif
