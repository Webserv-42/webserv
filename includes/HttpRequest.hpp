/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 13:29:17 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/16 15:48:53 by gafreire         ###   ########.fr       */
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
        Fíjate en el size_t& pos con el &. 
        ¿Por qué le paso pos por referencia? Porque parseRequestLine 
        tiene que decirle a parseHeaders dónde se quedó para que 
        pueda continuar leyendo desde allí. Si lo pasas sin &, 
        la posición se pierde.
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