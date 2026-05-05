/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:58:57 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:42:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ConfigData.hpp"
#include "bookstore.hpp"

enum ParseState {
	STATE_GLOBAL,
	STATE_SERVER,
	STATE_LOCATION
};
/*
	ConfigParser:
		reads the .conf file, validates its syntax, and translates it into
		configData structs.
*/

class ConfigParser 
{
	private:
		std::vector<ServerConfig> _servers;
		std::string trim(const std::string& str);
		std::vector<std::string> tokenize(const std::string& line);
		void initServerDefaults(ServerConfig& server);
		void initLocationDefaults(LocationConfig& location, const std::string& path);
		void applyServerErrorPages(ServerConfig& server);
		void printSummary() const;
		bool parseServerDirective(const std::vector<std::string>& tokens,
									ServerConfig& server);
		bool parseLocationDirective(const std::vector<std::string>& tokens,
									LocationConfig& location);
		bool validateServer(const ServerConfig& server);
		
	public:
		ConfigParser();
		~ConfigParser();
		
		bool parse(const std::string& filename);
		const std::vector<ServerConfig>& getServers() const;
};

#endif
