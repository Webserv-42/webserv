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

bool HttpHandler::saveUploadedFile(const std::string &filename, const std::string &fileContent, const LocationConfig &location)
{
	std::string fullPath = location.root + "/" + filename;
	std::ofstream outfile(fullPath.c_str(), std::ios::binary);
	if(!outfile.is_open())
		return false;
	outfile << fileContent;
	outfile.close();
	return true;
}

bool HttpHandler::deleteFile(const std::string &uri, const LocationConfig &location)
{
	std::string fullPath = location.root + uri;
	if(remove(fullPath.c_str()) == 0)
		return true;
	return false;
}

std::string HttpHandler::getErrorPageContent(int errorCode, const LocationConfig& location)
{
	std::map<int, std::string>::const_iterator it = location.errorPages.find(errorCode);
	if(it == location.errorPages.end())
		return "";
	std::string fullPath = location.root + it->second;
	std::ifstream file(fullPath.c_str());
	if(!file)
		return "";
	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();

}
