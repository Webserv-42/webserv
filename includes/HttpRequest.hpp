/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 13:29:17 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/14 16:17:06 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "bookstore.hpp"

class HttpRequest
{
private:
    // metodo
    std::string  _method;
    // URI 
    std::string  _uri;
    // version
    std::string  _httpVersion;
    // headers
    std::map<std::string, std::string>  _headers;
    // body
    std::string  _body;
    // estado
    enum ParseState
    {
        PARSING_INCOMPLETE,
        PARSING_COMPLETE,
        PARSING_ERROR 
    } _state;
public:
    HttpRequest();
    ~HttpRequest();
    void checkRequest();
    void getMethod(const std::string method);
    void getUri(const std::string uri);
    void getHttpVersion(const std::string httpVersion);
    void getHeaders(const std::map<std::string, std::string>  _headers);
    void getBody(const std::string  _body);
};

#endif