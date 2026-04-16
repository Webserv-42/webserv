/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 13:29:17 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:29:22 by gafreire         ###   ########.fr       */
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
    /*
        Uso el &, lo llamo por refencia para decirle a parseHeaders
        donde se quedo para que pueda continuar leyendo donde se quedo
        para que pueda continuar leyendo desde alli
    */
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