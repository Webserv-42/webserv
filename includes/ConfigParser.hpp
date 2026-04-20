/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:57 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/08 15:05:04 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

enum ParseState {
	STATE_GLOBAL,		// Outside any block
	STATE_SERVER,		// Inside a server { } block
	STATE_LOCATION		// Inside a location { } block (nested in server)
};

class ConfigParser {
private:
	std::vector<ServerConfig> _servers;
	std::string trim(const std::string& str);
	std::vector<std::string> tokenize(const std::string& line);
	// Processes a directive inside a server block (listen, server_name, etc.)
	bool parseServerDirective(const std::vector<std::string>& tokens,
								ServerConfig& server);

	// Processes a directive inside a location block (root, index, etc.)
	bool parseLocationDirective(const std::vector<std::string>& tokens,
								LocationConfig& location);

	// Validates that a ServerConfig has the minimum required fields (port, etc.)
	bool validateServer(const ServerConfig& server);

public:
	ConfigParser();
	~ConfigParser();

	// Reads and parses the config file, fills _servers
	bool parse(const std::string& filename);

	// Getter to retrieve the parsed servers
	const std::vector<ServerConfig>& getServers() const;
};

#endif
