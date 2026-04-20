#include "../includes/HttpHandler.hpp"

const LocationConfig *HttpHandler::findLocation(const std::string &uri, const ServerConfig &serverConf)
{
	const LocationConfig *bestMatch = NULL;
	size_t longestMatch = 0;
	for(size_t i = 0; i < serverConf.locations.size(); i++)
	{
		if(uri.find(serverConf.locations[i].path) == 0)
		{
			size_t currentLength = serverConf.locations[i].path.length();
			if(currentLength > longestMatch)
			{
				longestMatch = currentLength;
				bestMatch = &serverConf.locations[i];
			}
		}
	}
	return bestMatch;
}

std::string HttpHandler::getStaticFileContent(const std::string &uri, const LocationConfig &location)
{
	std::string	fullPath = location.root + uri;
	if(fullPath[fullPath.length() - 1] == '/')
	{
		fullPath = fullPath + location.index;
	}
	std::ifstream file(fullPath.c_str());
	if(!file)
		return "";
	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}
