/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:22:35 by alejagom          #+#    #+#             */
/*   Updated: 2026/04/22 15:21:34 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "bookstore.hpp"
#include "ConfigData.hpp"
class Client 
{
    public:
        int                 fd;
        std::string         buffer;
        const ServerConfig* config;
    
        Client() : fd(-1), buffer(""), config(NULL) {}
        Client(int fd) : fd(fd), buffer(""), config(NULL) {}
        ~Client() {}
};

#endif