/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 16:46:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/15 14:06:51 by gafreire         ###   ########.fr       */
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
    
}
std::string HttpRequest::getUri() const
{
    
}

std::string HttpRequest::getHttpVersion() const
{
    
}
std::map<std::string, std::string> HttpRequest::getHeaders()const
{
    
}

std::string HttpRequest::getBody() const
{
    
}

void HttpRequest::parse(const std::string& raw_data)
{
       
    size_t index_request = raw_data.find("\r\n");
    std::string request_line = raw_data.substr(0, index_request);
    std::cout << "Mi primera linea es: " << request_line << std::endl;
    
    size_t index_method = request_line.find(" ");
    std::string cpy_method = request_line.substr(0,index_method);
    std::cout << "Method: " << cpy_method << std::endl;

    size_t index_uri = cpy_method.find(" ", index_method + 1);
    std::string cpy_uri= cpy_method.substr(index_method,index_uri - (index_method + 1));
    std::cout << "Uri: " << cpy_uri << std::endl;

    std::string cpy_version = cpy_uri.substr(index_uri,strlen(cpy_uri));
    std::cout << "Version: " << cpy_version<< std::endl;

}