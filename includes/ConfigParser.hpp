/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:57 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/23 12:35:57 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

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

#endif