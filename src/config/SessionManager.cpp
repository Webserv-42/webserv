/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:44:33 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 11:44:38 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/SessionManager.hpp"

SessionManager::SessionManager(){}
SessionManager::~SessionManager(){}

bool SessionManager::isValidSession(const std::string &sessionId)
{
	if(_sessions.find(sessionId) != _sessions.end())
		return true;
	else
		return false;
}

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
	return newId;
}

