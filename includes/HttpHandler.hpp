/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/29 13:24:19 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"
#include "HttpRequest.hpp"
#include "CgiHandler.hpp"
#include "SessionManager.hpp"

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
		SessionManager _sessionManager;
		const LocationConfig *findLocation(const std::string &uri, const ServerConfig &serverConf);
		std::string getStaticFileContent(const std::string &uri, const LocationConfig &location);
		bool saveUploadedFile(const std::string &filename, const std::string &fileContent, const LocationConfig &location);
		bool deleteFile(const std::string &uri, const LocationConfig &location);
		std::string getErrorPageContent(int errorCode, const LocationConfig& location);
        std::string getMimeType(const std::string& filePath);
        const LocationConfig* matchLocation(const std::string& uri, const ServerConfig& serverConf);
        std::string buildErrorResponse(int statusCode, const ServerConfig* serverConf = NULL, const LocationConfig* loc = NULL);
        std::string generateDirectoryListing(const std::string& physicalPath, const std::string& currentUri);
        std::string handleGet(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri);
		std::string handlePost(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri);
		std::string handleDelete(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri);
        bool isMethodAllowed(const LocationConfig* loc, const std::string& method);
		bool processDirectory(std::string& filePath, const std::string& uri, const LocationConfig* loc, std::string& outResponse);
		std::string serveCgiIfMatch(const std::string& filePath, HttpRequest& req, const LocationConfig* loc);
		std::string serveStaticFile(const std::string& filePath, const ServerConfig& serverConf, const LocationConfig* loc);
		std::string manageSession(const std::string &cookieHeader);
    public:
        HttpHandler();
        ~HttpHandler();
        std::string handleRequest(HttpRequest& req, const ServerConfig& serverConf);
};

#endif
