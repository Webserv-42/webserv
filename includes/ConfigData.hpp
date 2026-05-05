/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:20 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:41:23 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGDATA_HPP
#define CONFIGDATA_HPP

/*
    ConfigData:
        we use structs instead of classes because these are data-transfer
        containers. They only store parsed .conf information for the
        server and HttpHandler to consume.
*/

#include "bookstore.hpp"

// Contract for routes (locations)
struct LocationConfig {
    std::string path;                               // Requested route, e.g. "/images/"
    std::string root;                               // Physical directory, e.g. "/var/www/html"
    std::string index;                              // Default file, e.g. "index.html"
    bool autoindex;                                 // Directory listing on/off
    std::vector<std::string> allowedMethods;        // Allowed methods: "GET", "POST", "DELETE"
    std::map<int, std::string> errorPages;          // Custom error pages, 404 -> "/404.html"
    std::string cgiExtension;                       // Extension that triggers CGI, e.g. ".php"
    std::string cgiPath;                            // Path to CGI binary, e.g. "/usr/bin/php-cgi"
    bool upload_enable;                             // Allow saving files from POST?
    std::string upload_store;                       // "www/uploads" -> Physical path for uploads
};

// Contract for the virtual server
struct ServerConfig
{
    int port;                                       // Listen port, e.g. 8080
    std::string host;                               // IP address, e.g. "127.0.0.1"
    std::string serverName;                         // Server name, e.g. "example.com"
    long clientMaxBodySize;                         // Max POST body size in bytes
    std::map<int, std::string> errorPages;          // Server-level error pages
    std::vector<LocationConfig> locations;          // List of routes for this server
};

#endif