/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerGet.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:25 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/29 13:36:19 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"


/*
    handleGet:
        Lógica maestra de peticiones GET
        1. Validar Permisos y obtener la ruta base
        2. Comprobar si la ruta es un Directorio
        3. Ejecutar CGI (Si coincide la extensión)
        4. Servir Archivo Estático
*/
std::string HttpHandler::handleGet(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri)
{
    std::string filePath;
    const LocationConfig* loc = matchLocation(uri, serverConf);
    
    if (loc != NULL) 
    {
        if (!isMethodAllowed(loc, req.getMethod()))
            return (buildErrorResponse(405, &serverConf, loc));
        filePath = loc->root + uri;
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
    std::string cgiResponse = serveCgiIfMatch(filePath, req, loc);
    if (!cgiResponse.empty()) 
        return (cgiResponse);
    return (serveStaticFile(filePath, serverConf, loc));
}

/*
    isMethodAllowed:
        1. Revisa si el bloque Location tiene una lista de métodos permitidos
        2. Si la lista está vacía, devuelve true por defecto
        3. Si la lista existe, devuelve true solo si encuentra el método actual en ella
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
        1. Verifica si el directorio tiene un archivo 'index' configurado
        2. Si el 'index' existe físicamente, actualiza la variable 'filePath' por referencia 
           para que el resto del código lo trate como una petición de archivo normal
        3. Si no hay 'index' pero 'autoindex' está activo, genera un listado HTML interactivo de la carpeta
        4. Retorna true si ya ha fabricado la respuesta final, o false si delegó en servir el archivo index
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
        1. Comprueba si el CGI está habilitado para esta ruta (comprobando loc->cgiExtension)
        2. Mira si las últimas letras del archivo solicitado coinciden con la extensión (ej. .py)
        3. Si hay coincidencia, instancia la clase CgiHandler pasándole el control absoluto
        4. Recibe el resultado del script, lo empaqueta con las cabeceras HTTP necesarias y lo devuelve
*/
std::string HttpHandler::serveCgiIfMatch(const std::string& filePath, HttpRequest& req, const LocationConfig* loc)
{
    if (loc == NULL || loc->cgiExtension.empty()) 
        return ("");
    if (filePath.length() >= loc->cgiExtension.length() && 
        filePath.substr(filePath.length() - loc->cgiExtension.length()) == loc->cgiExtension)
    {
        CgiHandler cgi;
        std::string scriptToRun = loc->cgiPath.empty() ? filePath : loc->cgiPath;
        std::string cgiOutput = cgi.executeCgi(scriptToRun, req);
        
        std::stringstream cgiResp;
        cgiResp << "HTTP/1.1 200 OK\r\n"
                << "Content-Length: " << cgiOutput.length() << "\r\n\r\n"
                << cgiOutput;
        return (cgiResp.str());
    }
    return ("");
}

std::string HttpHandler::serveStaticFile(const std::string& filePath, const ServerConfig& serverConf, const LocationConfig* loc)
{
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) 
        return (buildErrorResponse(404, &serverConf, loc));
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();        
    std::string contentType = getMimeType(filePath);    
    
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << content.length() << "\r\n\r\n"
             << content;
    return (response.str());
}