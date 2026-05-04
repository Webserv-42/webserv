/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerPost.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:39 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 18:45:39 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    handlePost:
        1. check the route configuration to see if it exists and allows POST
        2. check if the 'upload_enable' directive is enabled. If not, return 403 Forbidden.
        3. build a unique destination path for the file using a static counter
        4. open the file on the hard drive in binary format
        5. write the entire body of the HTTP request (req.getBody()) to the file
        6. if everything goes well, it returns a 201 Created code confirming the creation
*/
std::string HttpHandler::handlePost(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri, int* cgiPipeFd, int* cgiWriteFd)
{
    if (req.getBody().length() > (size_t)serverConf.clientMaxBodySize) 
        return (buildErrorResponse(413, &serverConf, NULL));
    
    const LocationConfig* loc = matchLocation(uri, serverConf);
    if (loc != NULL && !isMethodAllowed(loc, req.getMethod()))
        return (buildErrorResponse(405, &serverConf, loc));
    if (loc == NULL || loc->upload_enable == false) 
        return (buildErrorResponse(403, &serverConf, loc)); 
       
    std::string filePath = buildLocationPath(uri, loc);
    std::string cgiResponse = serveCgiIfMatch(filePath, req, loc, cgiPipeFd, cgiWriteFd);
    if (!cgiResponse.empty() || (cgiPipeFd != NULL && *cgiPipeFd != -1)) 
        return (cgiResponse);
    
    std::string uploadDir = loc->upload_store;
    std::string body = req.getBody();
    std::string filename = "";
    
    std::map<std::string, std::string> headers = req.getHeaders();
    if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
        
        size_t filenamePos = body.find("filename=\"");
        if (filenamePos != std::string::npos)
        {
            filenamePos += 10;
            size_t filenameEnd = body.find("\"", filenamePos);
            if (filenameEnd != std::string::npos)
                filename = body.substr(filenamePos, filenameEnd - filenamePos);
        }
        
        size_t contentStart = body.find("\r\n\r\n");
        if (contentStart != std::string::npos)
        {
            contentStart += 4;
            size_t contentEnd = body.find("\r\n------", contentStart);
            if (contentEnd == std::string::npos)
                contentEnd = body.find("\r\n--", contentStart);
                
            if (contentEnd != std::string::npos)
                body = body.substr(contentStart, contentEnd - contentStart);
        }
    }
    if (filename.empty())
    {
        std::stringstream ss;
        ss << "uploaded_" << time(NULL) << ".bin";
        filename = ss.str();
    }
    
    std::stringstream urlBuilder; 
    urlBuilder << uploadDir << (uploadDir[uploadDir.length() - 1] == '/' ? "" : "/") << filename;
    std::string fullPath = urlBuilder.str();
    if (!saveUploadedFile(fullPath, body))
        return (buildErrorResponse(500, &serverConf, loc));
    
    std::string resBody = "El archivo se ha subido correctamente al servidor Webserv!\n";
    std::stringstream response;
    response << "HTTP/1.1 201 Created\r\n"
             << "Location: " << fullPath << "\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << resBody.length() << "\r\n\r\n"
             << resBody;
             
    return (response.str());
}