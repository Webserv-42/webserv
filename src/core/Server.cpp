/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 14:55:56 by alejagom          #+#    #+#             */
/*   Updated: 2026/05/04 19:05:08 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

volatile sig_atomic_t Server::_stop = 0;

/*
	handleSigint:
		1. mark the server stop securely
*/
void Server::handleSigint(int)
{
	_stop = 1;
}

/*
	init:
		1. save the server settings
*/
void Server::init(const std::vector<ServerConfig>& configs)
{
    _configs = configs;
}


