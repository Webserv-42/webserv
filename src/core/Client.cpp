/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 15:34:52 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/03 20:57:19 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : fd(-1), serverFd(-1), buffer(""), bytesSend(0),
                   ContLength(0), state(READING_HEADERS), lastActivity(time(NULL)), keepAlive(false),
                   cgiBody(""), cgiBodySent(0), cgiWriteFd(-1)
{
	
}
Client::~Client() 
{
	
}

