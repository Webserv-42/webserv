/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:17:30 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/03 17:08:27 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigParser.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

// Example: "   listen 8080;   " → "listen 8080;"
//
// Why? .conf files have indentation (tabs/spaces).
// Without trim, "\tlisten" wouldn't match "listen".
//
// How it works:
//   1. find_first_not_of → finds the first non-space character
//   2. find_last_not_of  → finds the last non-space character
//   3. substr(start, length) → extracts the substring between the two

std::string ConfigParser::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

// Example: "allowed_methods GET POST DELETE;" → ["allowed_methods", "GET", "POST", "DELETE"]
//
// Why? Each directive has a format "key value1 value2...;".
// We need to separate the key (tokens[0]) from values (tokens[1], [2]...).
//
// How it works:
//   std::istringstream does exactly that: it reads word by word, skipping
//   spaces, exactly like cin >> variable.

std::vector<std::string> ConfigParser::tokenize(const std::string& line) {
	std::vector<std::string> tokens;
	std::istringstream iss(line);
	std::string word;

	while (iss >> word)
		tokens.push_back(word);
	return tokens;
}

// Receives the tokens from a line and fills the corresponding ServerConfig.
//
// Supported directives:
//   listen 8080;           → server.port
//   server_name localhost;  → server.serverName
//   client_max_body_size 1M; → server.clientMaxBodySize
//   error_page 404 /404.html; → server.locations[*].errorPages (global)

bool ConfigParser::parseServerDirective(const std::vector<std::string>& tokens,
										ServerConfig& server) {
	if (tokens.size() < 2) 
	{
		std::cerr << "[CONFIG ERROR] Incomplete directive in server block" << std::endl;
		return false;
	}

	std::string key = tokens[0];

	if (key == "listen") {
		// Convert port string → int
		// atoi() returns 0 if conversion fails, which is not a valid port
		server.port = std::atoi(tokens[1].c_str());
		if (server.port <= 0 || server.port > 65535) 
		{
			std::cerr << "[CONFIG ERROR] Invalid port: " << tokens[1] << std::endl;
			return false;
		}
	}
	else if (key == "server_name")
		server.serverName = tokens[1];
	else if (key == "host")
		server.host = tokens[1];
	else if (key == "client_max_body_size") 
	{
		server.clientMaxBodySize = std::atol(tokens[1].c_str());
		if (server.clientMaxBodySize <= 0) 
		{
			std::cerr << "[CONFIG ERROR] Invalid client_max_body_size: " << tokens[1] << std::endl;
			return false;
		}
	}
	else if (key == "error_page") 
	{
		// Format: error_page 404 /errors/404.html
		if (tokens.size() < 3) {
			std::cerr << "[CONFIG ERROR] error_page requires a code and a path" << std::endl;
			return false;
		}
		// Store error_pages at server level to apply them
		// to all locations that don't have their own
		int code = std::atoi(tokens[1].c_str());
		server.errorPages[code] = tokens[2];
	}
	else
		std::cerr << "[CONFIG WARNING] Unknown directive in server: " << key << std::endl;

	return true;
}

// Receives the tokens from a line and fills the corresponding LocationConfig.
//
// Supported directives:
//   root /var/www/html;         → location.root
//   index index.html;           → location.index
//   autoindex on;               → location.autoindex
//   allowed_methods GET POST;   → location.allowedMethods
//   cgi_extension .py;          → location.cgiExtension
//   cgi_path /usr/bin/python3;  → location.cgiPath
//   error_page 404 /404.html;   → location.errorPages

bool ConfigParser::parseLocationDirective(const std::vector<std::string>& tokens,
											LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "[CONFIG ERROR] Incomplete directive in location block" << std::endl;
		return false;
	}

	std::string key = tokens[0];

	if (key == "root")
		location.root = tokens[1];
	else if (key == "index")
		location.index = tokens[1];
	else if (key == "autoindex")
		// "on" → true, anything else → false
		location.autoindex = (tokens[1] == "on");
	else if (key == "upload_enable")
    	location.upload_enable = (tokens[1] == "on");
	else if (key == "upload_store")
	    location.upload_store = tokens[1];
	else if (key == "allowed_methods") {
		// Can have multiple values: GET POST DELETE
		// tokens[0] = "allowed_methods", tokens[1..n] = the methods
		location.allowedMethods.clear();
		for (size_t i = 1; i < tokens.size(); i++) {
			location.allowedMethods.push_back(tokens[i]);
		}
	}
	else if (key == "cgi_extension")
		location.cgiExtension = tokens[1];
	else if (key == "cgi_path")
		location.cgiPath = tokens[1];
	else if (key == "error_page") 
	{
		if (tokens.size() < 3) {
			std::cerr << "[CONFIG ERROR] error_page requires a code and a path" << std::endl;
			return false;
		}
		int code = std::atoi(tokens[1].c_str());
		location.errorPages[code] = tokens[2];
	}
	else if (key == "return")
	{
		if (tokens.size() < 3) {
			std::cerr << "[CONFIG ERROR] return requires a code and a URL" << std::endl;
			return false;
		}
		location.redirectCode = std::atoi(tokens[1].c_str());
		location.redirectUrl = tokens[2];
	}
	else
		std::cerr << "[CONFIG WARNING] Unknown directive in location: " << key << std::endl;
	return (true);
}

bool ConfigParser::validateServer(const ServerConfig& server) {
	if (server.port <= 0 || server.port > 65535) 
	{
		std::cerr << "[CONFIG ERROR] Server without a valid port" << std::endl;
		return false;
	}
	if (server.locations.empty()) 
	{
		std::cerr << "[CONFIG ERROR] Server without any location" << std::endl;
		return false;
	}
	return true;
}

// This is where everything connects. We read the file line by line,
// and depending on the CURRENT STATE (GLOBAL, SERVER, LOCATION), we interpret
// the line differently.
//
// Flow:
//   1. Open the file
//   2. For each line:
//      a. Trim + ignore empty/comment
//      b. Remove trailing ";"
//      c. Tokenize
//      d. Depending on state: process the line
//   3. Validate each server
//   4. Print a summary

bool ConfigParser::parse(const std::string& filename) {
	// --- Step 1: Open the file ---
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		std::cerr << "[CONFIG ERROR] Cannot open: " << filename << std::endl;
		return false;
	}

	// --- State machine variables ---
	ParseState state = STATE_GLOBAL;	// We start outside any block
	ServerConfig currentServer;			// The server currently being built
	LocationConfig currentLocation;		// The location currently being built
	std::string line;
	int lineNum = 0;					// For error messages

	// Initialize default server values
	currentServer.port = 0;
	currentServer.host = "0.0.0.0";
	currentServer.serverName = "";
	currentServer.clientMaxBodySize = 1048576; // 1MB default

	// --- Step 2: Read line by line ---
	while (std::getline(file, line)) 
	{
		lineNum++;

		// 2a. Trim whitespace
		line = trim(line);

		// 2b. Ignore empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		// 2c. Remove trailing ";" if present
		//     "listen 8080;" → "listen 8080"
		if (line[line.size() - 1] == ';')
			line = line.substr(0, line.size() - 1);

		// 2d. Tokenize the line into words
		std::vector<std::string> tokens = tokenize(line);
		if (tokens.empty())
			continue;

		if (state == STATE_GLOBAL) {
			if (tokens[0] == "server" && tokens.size() >= 2 && tokens[1] == "{") 
			{
				// Entering a server block
				state = STATE_SERVER;

				// Reset current server for a new block
				currentServer = ServerConfig();
				currentServer.port = 0;
				currentServer.host = "0.0.0.0";
				currentServer.serverName = "";
				currentServer.clientMaxBodySize = 1048576;
				currentServer.errorPages.clear();
			}
			// Special case: "server" alone on a line, "{" on the next
			else if (tokens[0] == "server" && tokens.size() == 1) {
				// We expect "{" on the next line or on this line
				// For simplicity, we also support this format
				state = STATE_SERVER;
				currentServer = ServerConfig();
				currentServer.port = 0;
				currentServer.host = "0.0.0.0";
				currentServer.serverName = "";
				currentServer.clientMaxBodySize = 1048576;
				currentServer.errorPages.clear();
			}
			else if (tokens[0] == "{") 
			{
				// Orphan "{" at global level → we're already opening a server
				// (case where "server" was on the previous line)
				// State is already STATE_SERVER, do nothing
			}
			else {
				std::cerr << "[CONFIG ERROR] Line " << lineNum
						  << ": expected 'server {', found: " << tokens[0] << std::endl;
				return false;
			}
		}

		else if (state == STATE_SERVER) {
			// Detect "}" → end of server block
			if (tokens[0] == "}") {
				// Redistribute server error pages to locations
				for (size_t i = 0; i < currentServer.locations.size(); ++i) {
					for (std::map<int, std::string>::const_iterator it = currentServer.errorPages.begin(); it != currentServer.errorPages.end(); ++it) 
					{
						if (currentServer.locations[i].errorPages.find(it->first) == currentServer.locations[i].errorPages.end()) {
							currentServer.locations[i].errorPages[it->first] = it->second;
						}
					}
				}

				// Validate the server before adding it
				if (!validateServer(currentServer)) {
					std::cerr << "[CONFIG ERROR] Validation failed (line "
							  << lineNum << ")" << std::endl;
					return false;
				}
				_servers.push_back(currentServer);
				state = STATE_GLOBAL;
			}
			// Detect "location /path/ {"
			else if (tokens[0] == "location") {
				if (tokens.size() < 3 || tokens[tokens.size() - 1] != "{") {
					// Maybe "location /path/" without "{" on the same line
					if (tokens.size() >= 2) {
						currentLocation = LocationConfig();
						currentLocation.path = tokens[1];
						currentLocation.autoindex = false;
						state = STATE_LOCATION;
						std::cout << "[CONFIG]   Location opened: "
								  << currentLocation.path << std::endl;
					} else {
						std::cerr << "[CONFIG ERROR] Line " << lineNum
								  << ": location without path" << std::endl;
						return false;
					}
				} else {
					// Normal format: "location /path/ {"
					currentLocation = LocationConfig();
					currentLocation.path = tokens[1];
					currentLocation.autoindex = false;
					state = STATE_LOCATION;
				}
			}
			// Otherwise it's a server directive
			else {
				if (!parseServerDirective(tokens, currentServer))
					return false;
			}
		}

		// ------ STATE LOCATION: we read route directives ------
		else if (state == STATE_LOCATION) {
			// Detect "}" → end of location block
			if (tokens[0] == "}") {
				currentServer.locations.push_back(currentLocation);
				state = STATE_SERVER;
				std::cout << "[CONFIG]   Location closed: " << currentLocation.path
						  << " (root=" << currentLocation.root << ")" << std::endl;
			}
			// Otherwise it's a location directive
			else {
				if (!parseLocationDirective(tokens, currentLocation))
					return false;
			}
		}
	}

	file.close();

	// --- Step 3: Final verification ---
	if (state != STATE_GLOBAL) {
		std::cerr << "[CONFIG ERROR] Unclosed brace(s) at end of file" << std::endl;
		return false;
	}

	if (_servers.empty()) {
		std::cerr << "[CONFIG ERROR] No server defined in " << filename << std::endl;
		return false;
	}
	for (size_t i = 0; i < _servers.size(); i++) {
		std::cout << "  Server " << i << ": " << _servers[i].host
				  << ":" << _servers[i].port
				  << " (name=" << _servers[i].serverName
				  << ", max_body=" << _servers[i].clientMaxBodySize
				  << ")" << std::endl;
		for (size_t j = 0; j < _servers[i].locations.size(); j++) {
			LocationConfig& loc = _servers[i].locations[j];
			std::cout << "    Location " << loc.path
					  << " -> root=" << loc.root;
			if (!loc.index.empty())
				std::cout << ", index=" << loc.index;
			if (!loc.cgiExtension.empty())
				std::cout << ", cgi=" << loc.cgiExtension;
			std::cout << ", methods=[";
			for (size_t k = 0; k < loc.allowedMethods.size(); k++) {
				if (k > 0) std::cout << " ";
				std::cout << loc.allowedMethods[k];
			}
			std::cout << "]" << std::endl;
		}
	}

	return true;
}

// ============================================================================
// getServers() — Returns the parsed servers
// ============================================================================

const std::vector<ServerConfig>& ConfigParser::getServers() const {
	return _servers;
}
