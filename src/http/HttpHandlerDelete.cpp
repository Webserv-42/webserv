/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerDelete.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:13:50 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/01 17:10:32 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    handleDelete:
        1. Revisa la configuración de la ruta para confirmar que DELETE está en 'allowed_methods'
        2. Calcula la ruta absoluta en disco del recurso solicitado
        3. Usa la llamada de C remove() para intentar eliminar físicamente el archivo
        4. Si remove() tiene éxito (devuelve 0), responde con un código 204 No Content
        5. Si remove() falla (archivo no existe o sin permisos), devuelve un 404 Not Found
*/
std::string HttpHandler::handleDelete(HttpRequest& req, const ServerConfig& serverConf, const std::string& uri)
{
    const LocationConfig* loc = matchLocation(uri, serverConf);
    if (loc != NULL && !loc->allowedMethods.empty()) 
    {
        bool isAllowed = false;
        for (size_t i = 0; i < loc->allowedMethods.size(); i++)
            if (loc->allowedMethods[i] == req.getMethod()) 
                isAllowed = true;
        if (!isAllowed) 
            return (buildErrorResponse(405, &serverConf, loc));
    }
    std::string filePath;
    if (loc != NULL)
        filePath = loc->root + uri;
    else
        filePath = "www" + uri;
    if (remove(filePath.c_str()) == 0) 
    {
        std::stringstream response;
        response << "HTTP/1.1 204 No Content\r\n\r\n";
        return (response.str());
    } 
    else
        return (buildErrorResponse(404, &serverConf, loc));
}