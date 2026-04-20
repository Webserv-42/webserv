/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:57 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:21:38 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

/*
	ConfigParser — State machine to parse an nginx-style .conf file.
	
	3 possible states:
	  GLOBAL   → outside any block, waiting for "server {"
	  SERVER   → inside a server { } block, reading server directives
	  LOCATION → inside a location { } block, reading route directives
	
	Each "{" goes one level deeper, each "}" goes back up.
*/

// The 3 states of the parser's state machine
enum ParseState {
	STATE_GLOBAL,		// Outside any block
	STATE_SERVER,		// Inside a server { } block
	STATE_LOCATION		// Inside a location { } block (nested in server)
};

class ConfigParser {
private:
	std::vector<ServerConfig> _servers;

	// === Parsing helpers ===
	// Removes spaces/tabs at the beginning and end of a string
	std::string trim(const std::string& str);

	// Splits a line into words (tokens) separated by spaces
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

    bool parse(const std::string& filename) {
        std::cout << "[DEV 2] Parseando archivo de configuracion: " << filename << std::endl;
        ServerConfig dummy;
        dummy.port = 8080;
        dummy.host = "127.0.0.1";
         LocationConfig locDummy;
        locDummy.path = "/";
        locDummy.root = "www";             // Carpeta real física
        locDummy.index = "index.html";     // Archivo primario
        locDummy.autoindex = true; 
        
        locDummy.upload_enable = true;          
        locDummy.upload_store = "www/uploads"; 
        locDummy.allowedMethods.push_back("GET"); 
        locDummy.allowedMethods.push_back("POST");
        locDummy.allowedMethods.push_back("DELETE");
        
        dummy.locations.push_back(locDummy);
        _servers.push_back(dummy);
        
        return (true);
    }
	// Reads and parses the config file, fills _servers
	bool parse(const std::string& filename);

	// Getter to retrieve the parsed servers
	const std::vector<ServerConfig>& getServers() const;
};

#endif