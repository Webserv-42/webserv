/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 13:29:17 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:44:04 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "bookstore.hpp"

/*
    HttpRequest:
        parses raw HTTP text into method, URI, version, headers, and body
        and exposes accessors for validated request data
*/
class HttpRequest
{
    private:
        std::string  _method;
        std::string  _uri;
        std::string  _httpVersion;
        std::map<std::string, std::string>  _headers;
        std::string  _body;
        enum ParseState
        {
            PARSING_INCOMPLETE,
            PARSING_COMPLETE,
            PARSING_ERROR 
        } _state;
        
        void parseRequestLine(const std::string& raw_data, size_t& pos);
        void parseHeaders(const std::string& raw_data, size_t& pos);
        void parseBody(const std::string& raw_data, size_t& pos);
        
    public:
        HttpRequest();
        ~HttpRequest();
        std::string getMethod() const;
        std::string getUri() const;
        std::string getHttpVersion() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        
        void checkRequest();
        void parse(const std::string& raw_data);
};

#endif