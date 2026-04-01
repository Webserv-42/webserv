/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:48 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/01 13:38:40 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigParser.hpp"
#include "../includes/Server.hpp"
#include <iostream>
#include <string>

/*
    Aqui recibimos argumentos, isntanciamos los modulos principales y conectarlos.
    No debe tener logica de red ni de parseo directa
*/

/*
    main:
        1. Determinar el archivo de configuracion por defecto
        2. Instancia y ejecutar el modulo de configuracion (ConfigParser)
        3. Instancia el modulo de Core/redes (Server) y pasarle la configuracion
        4.Iniciar el servidor (El loop comienza ahi)
*/

int main(int argc, char **argv) {
    std::string configFile = "conf/default.conf";
    
    if (argc == 2) {
        configFile = argv[1];
    } else if (argc > 2) {
        std::cerr << "Uso: ./webserv [archivo_de_configuracion]" << std::endl;
        return 1;
    }

    ConfigParser parser;
    if (!parser.parse(configFile)) {
        std::cerr << "Error crítico: No se pudo parsear " << configFile << std::endl;
        return 1;
    }

    Server webserv;
    try {
        webserv.init(parser.getServers());
        
        webserv.run();
    } catch (const std::exception& e) {
        std::cerr << "Excepción fatal en el servidor: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}