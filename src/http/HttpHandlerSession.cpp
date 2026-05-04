/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerSession.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 19:55:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 18:47:01 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    manageSession:
        1. look for session_id in the Cookie header
        2. validate the existing session or create a new one
*/
std::string HttpHandler::manageSession(const std::string &cookieHeader)
{
    size_t pos = cookieHeader.find("session_id=");
    if (pos != std::string::npos)
    {
        std::string sessionId = cookieHeader.substr(pos + 11);
        if (_sessionManager.isValidSession(sessionId))
            return ("");
    }
    return (_sessionManager.createSession());
}
