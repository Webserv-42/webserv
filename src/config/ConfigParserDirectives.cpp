/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserDirectives.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:25:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:15:21 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

/*
    parseServerDirective:
        1. process directives within a server block
        2. update the ServerConfig structure
*/
bool ConfigParser::parseServerDirective(const std::vector<std::string>& tokens,
									ServerConfig& server)
{
    if (tokens.size() < 2)
    {
        std::cerr << "[CONFIG ERROR] Incomplete directive in server block" << std::endl;
        return (false);
    }

    std::string key = tokens[0];

    if (key == "listen")
    {
        server.port = std::atoi(tokens[1].c_str());
        if (server.port <= 0 || server.port > 65535)
        {
            std::cerr << "[CONFIG ERROR] Invalid port: " << tokens[1] << std::endl;
            return (false);
        }
    }
    else if (key == "server_name")
        server.serverName = tokens[1];
    else if (key == "host")
        server.host = tokens[1];
    else if (key == "client_max_body_size")
    {
        server.clientMaxBodySize = std::atol(tokens[1].c_str());
        if (server.clientMaxBodySize <= 0)
        {
            std::cerr << "[CONFIG ERROR] Invalid client_max_body_size: " << tokens[1] << std::endl;
            return (false);
        }
    }
    else if (key == "error_page")
    {
        if (tokens.size() < 3)
        {
            std::cerr << "[CONFIG ERROR] error_page requires a code and a path" << std::endl;
            return (false);
        }
        int code = std::atoi(tokens[1].c_str());
        server.errorPages[code] = tokens[2];
    }
    else
        std::cerr << "[CONFIG WARNING] Unknown directive in server: " << key << std::endl;

    return (true);
}

/*
    parseLocationDirective:
        1. process directives within a location block
        2. update the LocationConfig structure
*/
bool ConfigParser::parseLocationDirective(const std::vector<std::string>& tokens,
										LocationConfig& location)
{
    if (tokens.size() < 2)
    {
        std::cerr << "[CONFIG ERROR] Incomplete directive in location block" << std::endl;
        return (false);
    }

    std::string key = tokens[0];

    if (key == "root")
        location.root = tokens[1];
    else if (key == "index")
        location.index = tokens[1];
    else if (key == "autoindex")
        location.autoindex = (tokens[1] == "on");
    else if (key == "upload_enable")
        location.upload_enable = (tokens[1] == "on");
    else if (key == "upload_store")
        location.upload_store = tokens[1];
    else if (key == "allowed_methods")
    {
        location.allowedMethods.clear();
        for (size_t i = 1; i < tokens.size(); i++)
            location.allowedMethods.push_back(tokens[i]);
    }
    else if (key == "cgi_extension")
        location.cgiExtension = tokens[1];
    else if (key == "cgi_path")
        location.cgiPath = tokens[1];
    else if (key == "error_page")
    {
        if (tokens.size() < 3)
        {
            std::cerr << "[CONFIG ERROR] error_page requires a code and a path" << std::endl;
            return (false);
        }
        int code = std::atoi(tokens[1].c_str());
        location.errorPages[code] = tokens[2];
    }
    else
        std::cerr << "[CONFIG WARNING] Unknown directive in location: " << key << std::endl;

    return (true);
}

/*
    validateServer:
        1. verify that the server has valid minimum fields
*/
bool ConfigParser::validateServer(const ServerConfig& server)
{
    if (server.port <= 0 || server.port > 65535)
    {
        std::cerr << "[CONFIG ERROR] Server without a valid port" << std::endl;
        return (false);
    }
    if (server.locations.empty())
    {
        std::cerr << "[CONFIG ERROR] Server without any location" << std::endl;
        return (false);
    }
    return (true);
}
