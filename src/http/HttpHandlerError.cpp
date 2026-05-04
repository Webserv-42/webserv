/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerError.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 19:55:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 12:47:12 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    getErrorPageContent:
        1. look for the file associated with the code in the errorPages of the Locationn
        2. it reads the content and returns it.
*/
std::string HttpHandler::getErrorPageContent(int errorCode, const LocationConfig& location)
{
    std::map<int, std::string>::const_iterator it = location.errorPages.find(errorCode);
    if (it == location.errorPages.end())
        return ("");
    std::string fullPath = location.root + it->second;
    return (readFileToString(fullPath));
}

/*
    loadCustomErrorContent:
        1. if there is a Location, look for errorPages in that block.
        2. otherwise, use the server's errorPages.
        3. returns content or an empty string if no file exists
*/
std::string HttpHandler::loadCustomErrorContent(int statusCode, const ServerConfig* serverConf, const LocationConfig* loc)
{
    if (loc != NULL)
    {
        std::map<int, std::string>::const_iterator it = loc->errorPages.find(statusCode);
        if (it != loc->errorPages.end())
            return (getErrorPageContent(statusCode, *loc));
        return ("");
    }
    if (serverConf != NULL)
    {
        std::map<int, std::string>::const_iterator it = serverConf->errorPages.find(statusCode);
        if (it != serverConf->errorPages.end())
        {
            std::string fullPath = it->second;
            if (!fullPath.empty() && fullPath[0] == '/')
            {
                std::string content = readFileToString(fullPath.substr(1));
                if (content.empty())
                    content = readFileToString(fullPath);
                return (content);
            }
            return (readFileToString(fullPath));
        }
    }
    return ("");
}

/*
    mapStatusCode:
        1. map the code to statusText and message
        2. if it doesn't exist, force 500
*/
void HttpHandler::mapStatusCode(int& statusCode, std::string& statusText, std::string& message)
{
    if (statusCode == 400)
    {
        statusText = "400 Bad Request";
        message = "The server could not understand the request.";
    }
    else if (statusCode == 403)
    {
        statusText = "403 Forbidden";
        message = "Access to this resource is denied.";
    }
    else if (statusCode == 404)
    {
        statusText = "404 Not Found";
        message = "Oops! The file you are looking for was not found on the server.";
    }
    else if (statusCode == 405)
    {
        statusText = "405 Method Not Allowed";
        message = "The HTTP method used is not allowed on this route.";
    }
    else if (statusCode == 413)
    {
        statusText = "413 Payload Too Large";
        message = "The body of the application is too large.";
    }
    else if (statusCode == 501)
    {
        statusText = "501 Not Implemented";
        message = "The server does not support the required functionality.";
    }
    else
    {
        statusText = "500 Internal Server Error";
        message = "An unexpected error unleashed chaos within the server.";
        statusCode = 500;
    }
}

/*
    buildDefaultErrorHtml:
        1. try loading the template from www/errors/default_error.html
        2. replace placeholders with real data
        3. if it fails, it generates the backup inline HTML
*/
std::string HttpHandler::buildDefaultErrorHtml(int statusCode, const std::string& statusText, const std::string& message)
{
    std::string templateHtml = readFileToString("www/errors/default_error.html");
    if (!templateHtml.empty())
    {
        std::stringstream codeStream;
        codeStream << statusCode;

        std::string titleText = statusText;
        if (statusText.length() > 4)
            titleText = statusText.substr(4);

        const std::string codeStr = codeStream.str();
        const std::string statusTextToken = "{{STATUS_TEXT}}";
        const std::string codeToken = "{{CODE}}";
        const std::string titleToken = "{{TITLE}}";
        const std::string messageToken = "{{MESSAGE}}";

        size_t pos = 0;
        while ((pos = templateHtml.find(statusTextToken, pos)) != std::string::npos)
        {
            templateHtml.replace(pos, statusTextToken.length(), statusText);
            pos += statusText.length();
        }
        pos = 0;
        while ((pos = templateHtml.find(codeToken, pos)) != std::string::npos)
        {
            templateHtml.replace(pos, codeToken.length(), codeStr);
            pos += codeStr.length();
        }
        pos = 0;
        while ((pos = templateHtml.find(titleToken, pos)) != std::string::npos)
        {
            templateHtml.replace(pos, titleToken.length(), titleText);
            pos += titleText.length();
        }
        pos = 0;
        while ((pos = templateHtml.find(messageToken, pos)) != std::string::npos)
        {
            templateHtml.replace(pos, messageToken.length(), message);
            pos += message.length();
        }

        return (templateHtml);
    }
    std::stringstream html;
    html << "<!DOCTYPE html>\n<html>\n"
         << "<head><title>" << statusText << "</title></head>\n"
         << "<body style=\"font-family: Arial, sans-serif; text-align: center; margin-top: 10%; background-color: #f4f4f4;\">\n"
         << "   <h1 style=\"font-size: 5rem; color: #ff4a4a; margin-bottom: 0;\">" << statusCode << "</h1>\n"
         << "   <h2 style=\"color: #333;\">" << statusText.substr(4) << "</h2>\n"
         << "   <p style=\"color: #666; margin-bottom: 30px;\">" << message << "</p>\n"
         << "   <hr style=\"width: 50%;\">\n"
         << "   <p style=\"color: #aaa; font-size: 0.8rem;\">Webserv / Proyecto 42</p>\n"
         << "</body>\n</html>";
    return (html.str());
}

/*
    buildHtmlResponse:
        1. build HTTP response with Content-Type text/html
*/
std::string HttpHandler::buildHtmlResponse(const std::string& statusText, const std::string& body)
{
    std::stringstream response;
    response << "HTTP/1.1 " << statusText << "\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "\r\n"
             << body;
    return (response.str());
}

/*
    buildErrorResponse:
        1. we choose the message according to the code
        2. we assemble the final HTML (custom or default)
        3. we construct the HTTP response
*/
std::string HttpHandler::buildErrorResponse(int statusCode, const ServerConfig* serverConf, const LocationConfig* loc)
{
    std::string customContent = loadCustomErrorContent(statusCode, serverConf, loc);

    std::string statusText;
    std::string message;
    mapStatusCode(statusCode, statusText, message);

    if (!customContent.empty())
        return (buildHtmlResponse(statusText, customContent));

    std::string htmlBody = buildDefaultErrorHtml(statusCode, statusText, message);
    return (buildHtmlResponse(statusText, htmlBody));
}
