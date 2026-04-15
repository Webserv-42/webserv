/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 16:46:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/15 17:42:18 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
    
}
HttpRequest::~HttpRequest()
{
    
}

void HttpRequest::checkRequest()
{
    
}
std::string HttpRequest::getMethod() const
{
    return (_method);
}
std::string HttpRequest::getUri() const
{
    return (_uri);
}

std::string HttpRequest::getHttpVersion() const
{
    return (_httpVersion);
}
std::map<std::string, std::string> HttpRequest::getHeaders()const
{
    return (_headers);
}

std::string HttpRequest::getBody() const
{
    return (_body);
}

void HttpRequest::parse(const std::string& raw_data)
{
       
    size_t index_request = raw_data.find("\r\n");
    if (index_request == std::string::npos) 
    {
        std::cout << "Error: Request_line mal formada.\n";
        _state = PARSING_ERROR;
        return;
    }
    std::string request_line = raw_data.substr(0, index_request);
    std::cout << "Mi primera linea es: " << request_line << std::endl;
    
    size_t index_method = request_line.find(" ");
    if (index_method == std::string::npos) 
    {
        std::cout << "Error: Request_line mal formada.\n";
        _state = PARSING_ERROR;
        return;
    }
    _method = request_line.substr(0,index_method);
    std::cout << "Method: " << _method << std::endl;

    size_t index_uri = request_line.find(" ", index_method + 1);
    if (index_uri == std::string::npos)
    {
        std::cout << "Error: Request_line mal formada.\n";
        _state = PARSING_ERROR;
        return;
    }
    _uri = request_line.substr(index_method + 1,index_uri - (index_method + 1));
    std::cout << "Uri: " << _uri << std::endl;

    _httpVersion = request_line.substr(index_uri + 1);
    std::cout << "Version: " << _httpVersion<< std::endl;

}