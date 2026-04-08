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
        _servers.push_back(dummy);
        return true;
    }

    const std::vector<ServerConfig>& getServers() const {
        return _servers;
    }
};

#endif