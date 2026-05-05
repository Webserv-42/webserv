/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 16:46:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 19:46:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <cctype>

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

/*
    toLower:
        return a lowercase copy of the input string.
*/
static std::string toLower(const std::string& value)
{
    std::string out = value;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return (out);
}

/*
    trimSpaces:
        remove leading and trailing whitespace.
*/
static std::string trimSpaces(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
        ++start;
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
        --end;
    return (value.substr(start, end - start));
}

/*
    normalizeHeaderKey:
        map common header names to canonical casing.
*/
static std::string normalizeHeaderKey(const std::string& key)
{
    std::string lowered = toLower(trimSpaces(key));
    if (lowered == "host")
        return ("Host");
    if (lowered == "content-length")
        return ("Content-Length");
    if (lowered == "content-type")
        return ("Content-Type");
    if (lowered == "transfer-encoding")
        return ("Transfer-Encoding");
    if (lowered == "connection")
        return ("Connection");
    if (lowered == "cookie")
        return ("Cookie");
    return (trimSpaces(key));
}

/*
    decodeChunkedBody:
        decode a chunked transfer-encoded body into a plain string.
*/
static std::string decodeChunkedBody(const std::string& rawBody)
{
    std::string decoded;
    size_t idx = 0;
    while (idx < rawBody.size())
    {
        size_t lineEnd = rawBody.find("\r\n", idx);
        if (lineEnd == std::string::npos)
            return ("");
        std::string sizeStr = rawBody.substr(idx, lineEnd - idx);
        size_t semicolon = sizeStr.find(';');
        if (semicolon != std::string::npos)
            sizeStr = sizeStr.substr(0, semicolon);
        if (sizeStr.empty())
            return ("");
        unsigned long size = std::strtoul(sizeStr.c_str(), NULL, 16);
        idx = lineEnd + 2;
        if (size == 0)
            return (decoded);
        if (idx + size + 2 > rawBody.size())
            return ("");
        decoded.append(rawBody.substr(idx, size));
        if (rawBody.compare(idx + size, 2, "\r\n") != 0)
            return ("");
        idx += size + 2;
    }
    return (decoded);
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
        _state = PARSING_ERROR;
        return;
    }
    std::string request_line = raw_data.substr(0, index_request);
    
    size_t index_method = request_line.find(" ");
    if (index_method == std::string::npos) 
    {
        _state = PARSING_ERROR;
        return;
    }
    _method = request_line.substr(0,index_method);

    size_t index_uri = request_line.find(" ", index_method + 1);
    if (index_uri == std::string::npos)
    {
        _state = PARSING_ERROR;
        return;
    }
    _uri = request_line.substr(index_method + 1,index_uri - (index_method + 1));

    _httpVersion = request_line.substr(index_uri + 1);
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
            std::string key = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 1);
            key = normalizeHeaderKey(key);
            value = trimSpaces(value);
            _headers[key] = value;
        }
        pos = end_of_line + 2;
    }
    _state = PARSING_COMPLETE;
}
void HttpRequest::parseBody(const std::string& raw_data, size_t& pos)
{
    if (_headers.find("Transfer-Encoding") != _headers.end())
    {
        std::string encoding = toLower(_headers["Transfer-Encoding"]);
        if (encoding.find("chunked") != std::string::npos)
        {
            std::string rawBody = raw_data.substr(pos);
            _body = decodeChunkedBody(rawBody);
            _state = PARSING_COMPLETE;
            return;
        }
    }
    if (_headers.find("Content-Length") == _headers.end())
    {
        _state = PARSING_COMPLETE;
        return;
    }
    std::string content_length_str = _headers["Content-Length"];
    int content_length = std::atoi(content_length_str.c_str());
    _body = raw_data.substr(pos, content_length);
    _state = PARSING_COMPLETE;
}