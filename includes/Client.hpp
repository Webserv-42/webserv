/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:22:35 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/23 18:35:27 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "bookstore.hpp"

enum ClientState
{
    READING_HEADERS,
    READING_BODY,
    PROCESSING,
    SENDING,
    DONE    
};

class Client
{
public:
    int         fd;		// File descriptor del cliente
    int		serverFd;	// que servidor acepto la conexión
    std::string buffer;		// Acumula lo que llega por recv
    std::string response;	// La respuesta construida
    size_t	bytesSend;	// Tiempo que lleva enviando datos
    size_t	ContLength;	// Header Content-Lenght
    ClientState	state;		// Estados para el HTTP
    HttpRequest	request;	// Variable para el request (GABRIEL).
    time_t      lastActivity;

    Client();
    Client(int fd);

    ~Client();
};

#endif