/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:48 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/14 18:54:05 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Server.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <string>

/*
    main:
        1. Determine the default configuration file
        2. Instantiate and run the configuration module (ConfigParser)
        3. Instantiate the core/network module (Server) and pass the config
        4. Start the server (the loop begins there)
*/

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));
    std::string configFile = "conf/default.conf";

    if (argc == 2)
        configFile = argv[1];
    else if (argc > 2) 
    {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return (1);
    }
    ConfigParser parser;
    if (!parser.parse(configFile)) 
    {
        std::cerr << "Critical error: could not parse " << configFile << std::endl;
        return (1);
    }

    Server webserv;

    try
    {
        webserv.init(parser.getServers());

        webserv.initSockets();

        webserv.run();

    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal server exception: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}
