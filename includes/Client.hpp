/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:22:35 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/01 16:40:11 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "bookstore.hpp"
#include "ConfigData.hpp"
#include "HttpRequest.hpp"

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
    int         fd;		// File descriptor del cliente
    int		serverFd;	// que servidor acepto la conexión
    std::string buffer;		// Acumula lo que llega por recv
    std::string	cookie;		// Variable para las cookies.
    std::string response;	// La respuesta construida
    size_t	bytesSend;	// Tiempo que lleva enviando datos
    size_t	ContLength;	// Header Content-Lenght
    ClientState	state;		// Estados para el HTTP
    HttpRequest	request;	// Variable para el request (GABRIEL).
    time_t      lastActivity;

    // Client();
    // Client(int fd);

    // ~Client();
    Client();
    Client(int fd);
    ~Client();

    const ServerConfig* config;

};


#endif