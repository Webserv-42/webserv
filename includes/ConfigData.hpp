/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:31:39 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGDATA_HPP
#define CONFIGDATA_HPP

/*
    Usamos structs en vez de clases porque estas son estructuras para
    transsferencia de datos. Solo almacenan la información parseada
    del .conf para que el server y el HttpHandler la usen.
*/

#include "bookstore.hpp"
// Contrato para las rutas (locations)
struct LocationConfig {
    std::string path;                               // La ruta solicitda ej: "/images/"
    std::string root;                               // Directorio físico ej: "/var/www/html"
    std::string index;                              // Archivo por defecto ej: "index.html"
    bool autoindex;                                 // Listado de directorios activado/desactivado
    std::vector<std::string> allowedMethods;        // Métodos permitidos: "GET", "POST", "DELETE"
    std::map<int, std::string> errorPages;          // Páginas de error personalizadas 404 -> "/404.html"
    std::string cgiExtension;                       // Extensión que dispara el CGI ej: ".php"
    std::string cgiPath;                            // Ruta al binario del CGI ej: "/usr/bin/php-cgi"
    bool upload_enable;                             // ¿Se permite guardar archivos desde POST?
    std::string upload_store;                       // "www/uploads" -> Ruta física donde se irán volcando
    int redirectCode;                               // 0 = sin redirección, 301 o 302
    std::string redirectUrl;                        // URL destino de la redirección
};

// Contrato para el servidor virtual
struct ServerConfig
{
    int port;                                       //Puerto de escucha ej: 8080
    std::string host;                               //Dirección IP ej: "127.0.0.1"
    std::string serverName;                         //Nombre del servidor ej: "ejemplo.com"
    long clientMaxBodySize;                         //Límite de tamaño para el body en POST en bytes
    std::map<int, std::string> errorPages;          // Páginas de error a nivel de servidor
    std::vector<LocationConfig> locations;          // Lista de rutas asociadas a este servidor
};

#endif