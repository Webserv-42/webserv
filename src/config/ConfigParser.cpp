/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:17:30 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:18:12 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

/*
	ConfigParser:
		- initialize the configuration parser
*/
ConfigParser::ConfigParser() 
{
	
}

/*
	~ConfigParser:
		- release resources if necessary
*/
ConfigParser::~ConfigParser() 
{
	
}

/*
    initServerDefaults:
        1. initializes server default values
*/
void ConfigParser::initServerDefaults(ServerConfig& server)
{
	server = ServerConfig();
	server.port = 0;
	server.host = "0.0.0.0";
	server.serverName = "";
	server.clientMaxBodySize = 1048576;
	server.errorPages.clear();
}

/*
    initLocationDefaults:
        1. initializes default location values

*/
void ConfigParser::initLocationDefaults(LocationConfig& location, const std::string& path)
{
	location = LocationConfig();
	location.path = path;
	location.autoindex = false;
	location.redirectCode = 0;
	location.redirectUrl = "";
}

/*
    applyServerErrorPages:
        1. propagate error_page from the server to locations
*/
void ConfigParser::applyServerErrorPages(ServerConfig& server)
{
	for (size_t i = 0; i < server.locations.size(); ++i)
	{
		for (std::map<int, std::string>::const_iterator it = server.errorPages.begin(); it != server.errorPages.end(); ++it)
		{
			if (server.locations[i].errorPages.find(it->first) == server.locations[i].errorPages.end())
				server.locations[i].errorPages[it->first] = it->second;
		}
	}
}

/*
    printSummary:
        1. print a summary of servers and locations
*/
void ConfigParser::printSummary() const
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "  Server " << i << ": " << _servers[i].host
				  << ":" << _servers[i].port
				  << " (name=" << _servers[i].serverName
				  << ", max_body=" << _servers[i].clientMaxBodySize
				  << ")" << std::endl;
		for (size_t j = 0; j < _servers[i].locations.size(); j++)
		{
			const LocationConfig& loc = _servers[i].locations[j];
			std::cout << "    Location " << loc.path
					  << " -> root=" << loc.root;
			if (!loc.index.empty())
				std::cout << ", index=" << loc.index;
			if (!loc.cgiExtension.empty())
				std::cout << ", cgi=" << loc.cgiExtension;
			std::cout << ", methods=[";
			for (size_t k = 0; k < loc.allowedMethods.size(); k++)
			{
				if (k > 0) std::cout << " ";
				std::cout << loc.allowedMethods[k];
			}
			std::cout << "]" << std::endl;
		}
	}
}

/*
	getServers:
		1. returns the list of parsed servers
*/
const std::vector<ServerConfig>& ConfigParser::getServers() const 
{
	return (_servers);
}
