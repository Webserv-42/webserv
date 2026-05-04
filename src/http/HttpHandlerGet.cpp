/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerGet.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:25 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 13:52:46 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"


/*
    handleGet:
        master logic for GET requests
        1. Validar Permisos y obtener la ruta base
        2. validate permissions and obtain the base path
        3. run CGI (If the extension matches)
        4. serve static file
*/
std::string HttpHandler::handleGet(HttpRequest& req, const ServerConfig& serverConf, 
    const std::string& uri, int* cgiPipeFd, int* cgiWriteFd)
{
    std::string filePath;
    const LocationConfig* loc = matchLocation(uri, serverConf);
    
    if (loc != NULL) 
    {
        if (!isMethodAllowed(loc, req.getMethod()))
            return (buildErrorResponse(405, &serverConf, loc));
        filePath = buildLocationPath(uri, loc);
    } 
    else 
    {
        filePath = "www" + uri;
        if (uri == "/") 
            filePath = "www/index.html";
    }
    struct stat s;
    if (stat(filePath.c_str(), &s) == 0 && (s.st_mode & S_IFDIR)) 
    {
        std::string dirResponse;
        bool handled = processDirectory(filePath, uri, loc, dirResponse);
        if (handled) 
            return (dirResponse);
    }
    std::string cgiResponse = serveCgiIfMatch(filePath, req, loc, cgiPipeFd, cgiWriteFd);
    if (!cgiResponse.empty()) 
        return (cgiResponse);
    return (serveStaticFile(filePath, serverConf, loc));
}

/*
    isMethodAllowed:
        1. check if the Location block has a list of allowed methods
        2. if the list is empty, it returns true by default.
        3. if the list exists, it returns true only if it finds the current method in it.
*/
bool HttpHandler::isMethodAllowed(const LocationConfig* loc, const std::string& method)
{
    if (loc->allowedMethods.empty())
        return (true);
    for (size_t i = 0; i < loc->allowedMethods.size(); i++)
        if (loc->allowedMethods[i] == method) 
            return (true);
    return (false);
}

/*
    processDirectory:
        1. check if the directory has an 'index' file configured
        2. if the 'index' physically exists, update the 'filePath' variable by reference
            so that the rest of the code treats it as a normal file request
        3. if there is no 'index' but 'autoindex' is active, it generates an interactive HTML listing of the folder
        4. returns true if it has already generated the final response, or false if it delegated serving the index file
*/
bool HttpHandler::processDirectory(std::string& filePath, const std::string& uri, const LocationConfig* loc, std::string& outResponse)
{
    if (loc == NULL) 
        return (false);
    if (loc->index != "") 
    {
        std::string indexFilePath = filePath;
        if (indexFilePath[indexFilePath.length() - 1] != '/') 
            indexFilePath += "/";
        indexFilePath += loc->index;
        struct stat sIdx;
        if (stat(indexFilePath.c_str(), &sIdx) == 0 && !(sIdx.st_mode & S_IFDIR))
        {
            filePath = indexFilePath;
            return (false);
        }
    }
    if (loc->autoindex) 
    {
        outResponse = generateDirectoryListing(filePath, uri);
        return (true); 
    } 
    
    outResponse = buildErrorResponse(403, NULL, loc);
    return (true);
}

/*
    serveCgiIfMatch:
        1. check if CGI is enabled for this path (by checking loc->cgiExtension)
        2. check if the last letters of the requested file match the extension (e.g., .py)
        3. if there is a match, instantiate the CgiHandler class, passing it absolute control.
        4. if it's CGI, it stores the read FD in cgiPipeFd and returns an empty string for asynchronous handling.
*/
std::string HttpHandler::serveCgiIfMatch(const std::string& filePath, HttpRequest& req, const LocationConfig* loc, int* cgiPipeFd, int* cgiWriteFd)
{
    if (loc == NULL || loc->cgiExtension.empty()) 
        return ("");
    if (filePath.length() >= loc->cgiExtension.length() && 
        filePath.substr(filePath.length() - loc->cgiExtension.length()) == loc->cgiExtension)
    {
        CgiHandler cgi;
        std::string scriptToRun;
        if (loc->cgiPath.empty()) 
            scriptToRun = filePath;
        else 
            scriptToRun = loc->cgiPath;
       int pipeFd = cgi.executeCgi(scriptToRun, filePath, req, cgiWriteFd);
        if (cgiPipeFd != NULL)
            *cgiPipeFd = pipeFd;
        return ("");
    }
    return ("");
}

std::string HttpHandler::serveStaticFile(const std::string& filePath, const ServerConfig& serverConf, const LocationConfig* loc)
{
    std::string content = readFileToString(filePath);
    if (content.empty())
        return (buildErrorResponse(404, &serverConf, loc));    
    std::string contentType = getMimeType(filePath);
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << content.length() << "\r\n\r\n"
             << content;
    return (response.str());
}