/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerSession.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 19:55:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 19:46:09 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    extractCookieValue:
        return the value for a named cookie or an empty string
*/
static std::string extractCookieValue(const std::string& cookieHeader, const std::string& name)
{
    std::string needle = name + "=";
    size_t pos = cookieHeader.find(needle);
    if (pos == std::string::npos)
        return ("");
    pos += needle.length();
    size_t end = cookieHeader.find(';', pos);
    std::string value = (end == std::string::npos)
        ? cookieHeader.substr(pos)
        : cookieHeader.substr(pos, end - pos);
    while (!value.empty() && value[0] == ' ')
        value.erase(0, 1);
    while (!value.empty() && value[value.length() - 1] == ' ')
        value.erase(value.length() - 1, 1);
    return (value);
}

/*
    manageSession:
        1. look for session_id in the Cookie header
        2. validate the existing session or create a new one
*/
std::string HttpHandler::manageSession(const std::string &cookieHeader)
{
    std::string sessionId = extractCookieValue(cookieHeader, "session_id");
    if (!sessionId.empty())
    {
        if (_sessionManager.isValidSession(sessionId))
            return ("");
    }
    return (_sessionManager.createSession());
}
