/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerPost.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:39 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/01 16:57:02 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    handlePost:
        1. Busca la configuración de la ruta para ver si existe y permite POST
        2. Verifica si la directiva 'upload_enable' está activada. Si no, devuelve 403 Forbidden
        3. Construye una ruta única de destino para el archivo usando un contador estático
        4. Abre el archivo en el disco duro de forma binaria
        5. Escribe el cuerpo entero de la petición HTTP (req.getBody()) en el archivo
        6. Si todo sale bien, devuelve un código 201 Created confirmando la creación
*/
std::string HttpHandler::handlePost(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri, int* cgiPipeFd)
{
    if (req.getBody().length() > (size_t)serverConf.clientMaxBodySize) 
    {
        std::cout << "[HTTP] Rechazando POST: tamaño " << req.getBody().length() 
                  << " supera el límite de " << serverConf.clientMaxBodySize << std::endl;
        return (buildErrorResponse(413, &serverConf, NULL));
    }
    
    const LocationConfig* loc = matchLocation(uri, serverConf);
    if (loc == NULL || loc->upload_enable == false) 
        return (buildErrorResponse(403, &serverConf, loc)); 
        
    // Comprobamos si la petición POST va dirigida a un script CGI
    std::string filePath = loc->root + uri;
    std::string cgiResponse = serveCgiIfMatch(filePath, req, loc, cgiPipeFd);
    if (!cgiResponse.empty() || (cgiPipeFd != NULL && *cgiPipeFd != -1)) 
        return (cgiResponse);
    
    std::string uploadDir = loc->upload_store;
    static int fileCounter = 0;
    
    std::stringstream urlBuilder; 
    urlBuilder << uploadDir << (uploadDir[uploadDir.length() - 1] == '/' ? "" : "/") << "uploaded_" << fileCounter << ".bin";
    std::string fullPath = urlBuilder.str();
    
    fileCounter++; 
    std::ofstream outFile(fullPath.c_str(), std::ios::binary);
    if (!outFile.is_open())
        return (buildErrorResponse(500, &serverConf, loc)); 
    
    outFile << req.getBody(); 
    outFile.close();
    
    std::string resBody = "El archivo se ha subido correctamente al servidor Webserv!\n";
    std::stringstream response;
    response << "HTTP/1.1 201 Created\r\n"
             << "Location: " << fullPath << "\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << resBody.length() << "\r\n\r\n"
             << resBody;
             
    return (response.str());
}