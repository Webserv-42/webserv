/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 16:46:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/16 15:59:42 by gafreire         ###   ########.fr       */
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
    size_t pos = 0;
    parseRequestLine(raw_data, pos);
    if (_state == PARSING_ERROR)
        return;
    parseHeaders(raw_data, pos);
    if (_state == PARSING_ERROR)
        return;
    parseBody(raw_data, pos); 
}

void HttpRequest::parseRequestLine(const std::string& raw_data, size_t& pos)
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
    pos = index_request + 2;
    _state = PARSING_COMPLETE;
}

void HttpRequest::parseHeaders(const std::string& raw_data, size_t& pos)
{    
    while (true)
    {
        size_t end_of_line = raw_data.find("\r\n", pos);
        if (end_of_line == std::string::npos)
        {
            _state = PARSING_ERROR;
            return;
        }
        std::string header_line = raw_data.substr(pos, end_of_line - pos);
        if (header_line.empty())
        {
            pos = end_of_line + 2;
            break;
        }
        size_t colon = header_line.find(":");
        if (colon != std::string::npos)
        {
            std::string key   = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 2);
            _headers[key] = value;
            std::cout << "Header → " << key << " : " << value << std::endl;
        }
        pos = end_of_line + 2;
    }
    _state = PARSING_COMPLETE;
    std::cout << "Headers parseados correctamente." << std::endl;
}

void HttpRequest::parseBody(const std::string& raw_data, size_t& pos)
{
    if (_headers.find("Content-Length") == _headers.end())
    {
        _state = PARSING_COMPLETE;
        return;
    }
    std::string content_length_str = _headers["Content-Length"];
    int content_length = std::atoi(content_length_str.c_str());
    _body = raw_data.substr(pos, content_length);
    std::cout << "Body: " << _body << std::endl;
    _state = PARSING_COMPLETE;
}