/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:44:33 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:21:54 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SessionManager.hpp"

/*
	SessionManager:
		1. initialize the session manager
*/
SessionManager::SessionManager()
{
	
}

/*
	~SessionManager:
		1. free up resources if necessary
*/
SessionManager::~SessionManager()
{

}

/*
	isValidSession:
		1. check if the ID exists on the map
*/
bool SessionManager::isValidSession(const std::string &sessionId)
{
	if(_sessions.find(sessionId) != _sessions.end())
		return (true);
	else
		return (false);
}

/*
	createSession:
		1. generates a unique random ID
		2. save the session in memory
*/
std::string SessionManager::createSession()
{
	std::string newId;
	do
	{
		std::ostringstream ss;
		ss << rand();
		newId = ss.str();
	} while (isValidSession(newId) == true);
	_sessions[newId] = "connected";
	return (newId);
}

