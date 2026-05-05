/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserParse.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:25:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:18:02 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

/*
    parse:
        1. read the file and apply the state machine
        2. build servers and locations
        3. validate and display a summary
*/
bool ConfigParser::parse(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "[CONFIG ERROR] Cannot open: " << filename << std::endl;
        return (false);
    }

    ParseState state = STATE_GLOBAL;
    ServerConfig currentServer;
    LocationConfig currentLocation;
    std::string line;
    int lineNum = 0;

    initServerDefaults(currentServer);

    while (std::getline(file, line))
    {
        lineNum++;
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (line[line.size() - 1] == ';')
            line = line.substr(0, line.size() - 1);

        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty())
            continue;

        if (state == STATE_GLOBAL)
        {
            if (tokens[0] == "server" && tokens.size() >= 2 && tokens[1] == "{")
            {
                state = STATE_SERVER;
                initServerDefaults(currentServer);
            }
            else if (tokens[0] == "server" && tokens.size() == 1)
            {
                state = STATE_SERVER;
                initServerDefaults(currentServer);
            }
            else if (tokens[0] == "{")
            {
            }
            else
            {
                std::cerr << "[CONFIG ERROR] Line " << lineNum
                          << ": expected 'server {', found: " << tokens[0] << std::endl;
                return (false);
            }
        }
        else if (state == STATE_SERVER)
        {
            if (tokens[0] == "}")
            {
                applyServerErrorPages(currentServer);
                if (!validateServer(currentServer))
                {
                    std::cerr << "[CONFIG ERROR] Validation failed (line "
                              << lineNum << ")" << std::endl;
                    return (false);
                }
                _servers.push_back(currentServer);
                state = STATE_GLOBAL;
            }
            else if (tokens[0] == "location")
            {
                if (tokens.size() < 3 || tokens[tokens.size() - 1] != "{")
                {
                    if (tokens.size() >= 2)
                    {
                        initLocationDefaults(currentLocation, tokens[1]);
                        state = STATE_LOCATION;
                        std::cout << "[CONFIG]   Location opened: "
                                  << currentLocation.path << std::endl;
                    }
                    else
                    {
                        std::cerr << "[CONFIG ERROR] Line " << lineNum
                                  << ": location without path" << std::endl;
                        return (false);
                    }
                }
                else
                {
                    initLocationDefaults(currentLocation, tokens[1]);
                    state = STATE_LOCATION;
                }
            }
            else
            {
                if (!parseServerDirective(tokens, currentServer))
                    return (false);
            }
        }
        else if (state == STATE_LOCATION)
        {
            if (tokens[0] == "}")
            {
                currentServer.locations.push_back(currentLocation);
                state = STATE_SERVER;
                std::cout << "[CONFIG]   Location closed: " << currentLocation.path
                          << " (root=" << currentLocation.root << ")" << std::endl;
            }
            else
            {
                if (!parseLocationDirective(tokens, currentLocation))
                    return (false);
            }
        }
    }

    file.close();

    if (state != STATE_GLOBAL)
    {
        std::cerr << "[CONFIG ERROR] Unclosed brace(s) at end of file" << std::endl;
        return (false);
    }

    if (_servers.empty())
    {
        std::cerr << "[CONFIG ERROR] No server defined in " << filename << std::endl;
        return (false);
    }
    printSummary();

    return (true);
}
