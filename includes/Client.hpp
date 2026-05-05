/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:22:35 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/05 17:39:39 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "bookstore.hpp"
#include "ConfigData.hpp"
#include "HttpRequest.hpp"

/*
    client:
        represents a connected client and its request/response state
        stores buffers, parsing state, timeouts, and CGI I/O info.
*/
enum ClientState
{
    READING_HEADERS,
    READING_BODY,
    PROCESSING,
    CGI_WAITING,
    SENDING,
    DONE    
};

class Client
{
    public:
        Client();
        Client(int fd);
        ~Client();
        
        int         fd;
        int		serverFd;
        std::string buffer;
        std::string	cookie;
        std::string response;
        size_t	bytesSend;
        size_t	ContLength;
        ClientState	state;
        HttpRequest	request;
        time_t      lastActivity;
        bool        keepAlive;
        std::string cgiBody;
        size_t      cgiBodySent;
        int         cgiWriteFd;
        
        const ServerConfig* config;
    
};

#endif