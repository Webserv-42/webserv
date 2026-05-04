/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandlerUtils.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 19:55:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 18:53:01 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
    readFileToString:
        1. open the file in read mode
        2. returns the entire content as a string
*/
std::string HttpHandler::readFileToString(const std::string& fullPath)
{
    std::ifstream file(fullPath.c_str());
    if (!file)
        return ("");
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return (buffer.str());
}

/*
    saveUploadedFile:
        1. open the destination file in binary mode
        2. write the received content
*/
bool HttpHandler::saveUploadedFile(const std::string &fullPath, const std::string &fileContent)
{
    std::ofstream outfile(fullPath.c_str(), std::ios::binary);
    if (!outfile.is_open())
        return (false);
    outfile << fileContent;
    outfile.close();
    return (true);
}

/*
    deleteFile:
        1. delete the physical file from the disk
        2. returns true if remove() is successful
*/
bool HttpHandler::deleteFile(const std::string &fullPath)
{
    if (remove(fullPath.c_str()) == 0)
        return (true);
    return (false);
}

/*
    getMimeType:
        1. detects file extension
        2. returns the corresponding MIME type
*/
std::string HttpHandler::getMimeType(const std::string& filePath)
{
    size_t pos;
    std::string ext;

    pos = filePath.find_last_of('.');
    if (pos == std::string::npos)
        return ("text/plain");
    ext = filePath.substr(pos);
    if (ext == ".html" || ext == ".htm")
        return ("text/html");
    if (ext == ".css")
        return ("text/css");
    if (ext == ".js")
        return ("application/javascript");
    if (ext == ".jpg" || ext == ".jpeg")
        return ("image/jpeg");
    if (ext == ".png")
        return ("image/png");
    if (ext == ".gif")
        return ("image/gif");
    if (ext == ".ico")
        return ("image/x-icon");
    return ("text/plain");
}

/*
    buildLocationPath:
        1. cut the location prefix in the URI
        2. join root with the resulting relative path
*/
std::string HttpHandler::buildLocationPath(const std::string& uri, const LocationConfig* loc)
{
    if (loc == NULL)
        return ("");

    std::string relative = uri;
    if (uri.find(loc->path) == 0)
        relative = uri.substr(loc->path.length());

    std::string fullPath = loc->root;
    if (!relative.empty())
    {
        if (!fullPath.empty() && fullPath[fullPath.length() - 1] == '/' && relative[0] == '/')
            relative = relative.substr(1);
        else if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/' && relative[0] != '/')
            fullPath += "/";
        fullPath += relative;
    }
    return (fullPath);
}

/*
    matchLocation:
        1. we go through all the Locations on the server
        2. we select the longest path that matches
*/
const LocationConfig* HttpHandler::matchLocation(const std::string& uri, const ServerConfig& serverConf)
{
    const LocationConfig* bestMatch;
    size_t longestMatch;

    bestMatch = NULL;
    longestMatch = 0;

    for (size_t i = 0; i < serverConf.locations.size(); ++i)
    {
        std::string locPath = serverConf.locations[i].path;
        if (uri.find(locPath) == 0)
        {
            if (locPath.length() >= longestMatch)
            {
                longestMatch = locPath.length();
                bestMatch = &serverConf.locations[i];
            }
        }
    }
    return (bestMatch);
}

/*
    generateDirectoryListing:
        1. open the directory and build an HTML file with links
        2. iterate through the entries and add each file.
        3. returns the complete HTTP response
*/
std::string HttpHandler::generateDirectoryListing(const std::string& physicalPath, const std::string& currentUri)
{
    DIR* dir;
    struct dirent* entry;
    std::stringstream html;

    dir = opendir(physicalPath.c_str());
    if (dir == NULL)
        return (buildErrorResponse(403));

    html << "<!DOCTYPE html>\n<html>\n<head><title>Index of " << currentUri << "</title></head>\n"
         << "<body style=\"font-family: monospace;\">\n"
         << "<h1>Index of " << currentUri << "</h1><hr>\n"
         << "<ul style=\"list-style-type: none; padding: 0;\">\n";
    while ((entry = readdir(dir)) != NULL)
    {
        std::string fileName = entry->d_name;
        if (fileName == ".")
            continue;
        std::string displayName = fileName;
        if (entry->d_type == DT_DIR)
            displayName += "/";
        html << "  <li style=\"margin: 5px 0;\"><a href=\"" << currentUri
             << (currentUri[currentUri.length() - 1] == '/' ? "" : "/")
             << displayName << "\">" << displayName << "</a></li>\n";
    }
    closedir(dir);
    html << "</ul>\n<hr>\n</body>\n</html>";
    std::string htmlBody = html.str();
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << htmlBody.length() << "\r\n"
             << "\r\n"
             << htmlBody;

    return (response.str());
}
