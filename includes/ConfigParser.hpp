/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:57 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:31:45 by gafreire         ###   ########.fr       */
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
/*
    Leer el archivo .conf de texto, validar su sintaxis y traducirlo a los structs
    de ConfigData

    Aqui trabaja " "
*/

/*
    class ConfigParser:
        - Se lee el archivo .conf y llena _servers
        - los datos del serv y port son una plantilla MODIFICAR!!
*/
class ConfigParser {
private:
    std::vector<ServerConfig> _servers;

public:
    ConfigParser() {}
    ~ConfigParser() {}

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

    const std::vector<ServerConfig>& getServers() const {
        return _servers;
    }
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
