/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:48 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/15 17:35:31 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Server.hpp"
#include "HttpRequest.hpp"
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

// int main(int argc, char **argv) {
//     std::string configFile = "conf/default.conf";
    
//     if (argc == 2) {
//         configFile = argv[1];
//     } else if (argc > 2) {
//         std::cerr << "Uso: ./webserv [archivo_de_configuracion]" << std::endl;
//         return 1;
//     }

//     ConfigParser parser;
//     if (!parser.parse(configFile)) {
//         std::cerr << "Error crítico: No se pudo parsear " << configFile << std::endl;
//         return 1;
//     }

//     Server webserv;
//     try {
//         webserv.init(parser.getServers());
        
//         webserv.run();
//     } catch (const std::exception& e) {
//         std::cerr << "Excepción fatal en el servidor: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }

int main(int argc, char **argv) 
{
     // ---- ZONA DE PRUEBAS DE HTTPREQUEST ----
    std::cout << "--- INICIANDO TEST DE PARSE ---" << std::endl;
    
    HttpRequest testReq;
    std::string mock_request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
    // Llamamos a tu funcion
    testReq.parse(mock_request);
    
    std::cout << "--- FIN DEL TEST ---" << std::endl;
    
    // Matamos el programa aqui para que no arranque el Server de momento
    return 0; 
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
        // 🔹 1. pasar configs
        webserv.init(parser.getServers());

        // 🔥 2. CREAR SOCKETS (LO QUE FALTABA)
        webserv.initSockets();

        // 🔥 3. ARRANCAR CORE
        webserv.run();

    } catch (const std::exception& e) {
        std::cerr << "Excepción fatal en el servidor: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}