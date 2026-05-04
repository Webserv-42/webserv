/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerDelete.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:50 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 12:35:24 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    handleDelete:
        1. Check the route configuration to confirm that DELETE is in 'allowed_methods'
        2. Calculate the absolute disk path of the requested resource
        3. Use the deleteFile() to attempt to physically delete the file
        4. If deleteFile() is successful (returns 0), it responds with a 204 No Content code.
        5. If deleteFile() fails (file does not exist or lacks permissions), it returns a 404 Not Found.
*/
std::string HttpHandler::handleDelete(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri)
{
    const LocationConfig* loc = matchLocation(uri, serverConf);
    if (loc != NULL && !isMethodAllowed(loc, req.getMethod()))
        return (buildErrorResponse(405, &serverConf, loc));
    std::string filePath;
    if (loc != NULL)
        filePath = buildLocationPath(uri, loc);
    else
        filePath = "www" + uri;
    if (deleteFile(filePath))
    {
        std::stringstream response;
        response << "HTTP/1.1 204 No Content\r\n\r\n";
        return (response.str());
    }
    else
        return (buildErrorResponse(404, &serverConf, loc));
}