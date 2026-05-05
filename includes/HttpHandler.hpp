/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:42:56 by gafreire         ###   ########.fr       */
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
    HttpHandler:
        receives the raw HTTP request, parses it, applies routing rules
        (ServerConfig), executes CGI when needed via fork/execve,
        and builds the final HTTP response string
*/

class HttpHandler
{
	private:
		SessionManager _sessionManager;
		bool saveUploadedFile(const std::string &fullPath, const std::string &fileContent);
		bool deleteFile(const std::string &fullPath);
        std::string readFileToString(const std::string& fullPath);
        std::string loadCustomErrorContent(int statusCode, const ServerConfig* serverConf, const LocationConfig* loc);
        void mapStatusCode(int& statusCode, std::string& statusText, std::string& message);
        std::string buildDefaultErrorHtml(int statusCode, const std::string& statusText, const std::string& message);
        std::string buildHtmlResponse(const std::string& statusText, const std::string& body);
		std::string getErrorPageContent(int errorCode, const LocationConfig& location);
        std::string getMimeType(const std::string& filePath);
        std::string buildLocationPath(const std::string& uri, const LocationConfig* loc);
        const LocationConfig* matchLocation(const std::string& uri, const ServerConfig& serverConf);
        std::string buildErrorResponse(int statusCode, const ServerConfig* serverConf = NULL, const LocationConfig* loc = NULL);
        std::string generateDirectoryListing(const std::string& physicalPath, const std::string& currentUri);
        std::string handleGet(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri, int* cgiPipeFd, int* cgiWriteFd);
        std::string handlePost(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri, int* cgiPipeFd, int* cgiWriteFd);
		std::string handleDelete(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri);
        bool isMethodAllowed(const LocationConfig* loc, const std::string& method);
		bool processDirectory(std::string& filePath, const std::string& uri, const LocationConfig* loc, std::string& outResponse);
        std::string serveCgiIfMatch(const std::string& filePath, HttpRequest& req, const LocationConfig* loc, int* cgiPipeFd, int* cgiWriteFd);
		std::string serveStaticFile(const std::string& filePath, const ServerConfig& serverConf, const LocationConfig* loc);
		std::string manageSession(const std::string &cookieHeader);
        
    public:
        HttpHandler();
        ~HttpHandler();
        std::string handleRequest(HttpRequest& req, const ServerConfig& serverConf, int* cgiPipeFd = NULL, int* cgiWriteFd = NULL);
};

#endif
