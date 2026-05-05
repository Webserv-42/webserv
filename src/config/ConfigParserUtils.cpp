/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserUtils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:25:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:19:35 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigParser.hpp"

/*
    trim:
        1. remove spaces at the beginning and end
        2. it restores a clean line
*/
std::string ConfigParser::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return ("");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (str.substr(start, end - start + 1));
}

/*
    tokenize:
        1. divide a line into tokens separated by spaces
*/
std::vector<std::string> ConfigParser::tokenize(const std::string& line)
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;

    while (iss >> word)
        tokens.push_back(word);
    return (tokens);
}
