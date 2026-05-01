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
    std::string body = req.getBody();
    std::string filename = "";
    
    // 1. Intentamos leer si es un "multipart/form-data" para sacar el nombre y el archivo real
    std::map<std::string, std::string> headers = req.getHeaders();
    if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
        // Buscamos el nombre del archivo en la etiqueta: filename="nombre.jpg"
        size_t filenamePos = body.find("filename=\"");
        if (filenamePos != std::string::npos)
        {
            filenamePos += 10; // Saltamos los 10 caracteres de: filename="
            size_t filenameEnd = body.find("\"", filenamePos);
            if (filenameEnd != std::string::npos)
                filename = body.substr(filenamePos, filenameEnd - filenamePos);
        }
        
        // Buscamos dónde termina el "papel de regalo" y empieza el archivo binario real
        // Siempre hay un salto de línea doble (\r\n\r\n) antes de la foto o archivo.
        size_t contentStart = body.find("\r\n\r\n");
        if (contentStart != std::string::npos)
        {
            contentStart += 4; // Saltamos esos 4 bytes invisibles de los saltos de línea
            
            // Buscamos el final del archivo (justo antes del texto de cierre o "boundary")
            size_t contentEnd = body.find("\r\n------", contentStart);
            if (contentEnd == std::string::npos)
                contentEnd = body.find("\r\n--", contentStart);
                
            if (contentEnd != std::string::npos)
            {
                // Recortamos el cuerpo para quedarnos SOLO con los datos puros
                body = body.substr(contentStart, contentEnd - contentStart);
            }
        }
    }
    
    // 2. Si no pudimos encontrar un nombre (o no era multipart), usamos el tiempo para no chocar
    if (filename.empty())
    {
        std::stringstream ss;
        ss << "uploaded_" << time(NULL) << ".bin";
        filename = ss.str();
    }
    
    // 3. Juntamos la carpeta con el nombre (Ej: www/uploads/ + mi_foto.jpg)
    std::stringstream urlBuilder; 
    urlBuilder << uploadDir << (uploadDir[uploadDir.length() - 1] == '/' ? "" : "/") << filename;
    std::string fullPath = urlBuilder.str();
    
    // 4. Guardamos el archivo final limpio
    std::ofstream outFile(fullPath.c_str(), std::ios::binary);
    if (!outFile.is_open())
        return (buildErrorResponse(500, &serverConf, loc)); 
    
    outFile << body; 
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