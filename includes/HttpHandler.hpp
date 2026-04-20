/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:59:47 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/08 15:05:18 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

class HttpHandler
{
	private:
		const LocationConfig *findLocation(const std::string &uri, const ServerConfig &serverConf);
		std::string getStaticFileContent(const std::string &uri, const LocationConfig &location);
    public:
        HttpHandler() {}
        ~HttpHandler() {}

        std::string handleRequest(const std::string& rawRequest, const ServerConfig& serverConf) {
            (void)rawRequest;
            (void)serverConf;

            std::cout << "[DEV 3] Processing HTTP request and generating response..." << std::endl;


            return "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello Webserv";
        }
};

#endif
