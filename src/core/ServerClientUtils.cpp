/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerClientUtils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 20:05:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 19:48:19 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerClientUtils.hpp"
#include <cctype>
#include <cstdlib>

namespace ServerClientUtils
{

/*
    toLower:
        return a lowercase copy of the input string.
*/
std::string toLower(const std::string& value)
{
    std::string out = value;
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    return (out);
}

/*
    trimSpaces:
        remove leading and trailing whitespace.
*/
std::string trimSpaces(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
        ++start;
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
        --end;
    return (value.substr(start, end - start));
}

/*
    getHeaderValue:
        find a header value by name in a raw header block.
*/
bool getHeaderValue(const std::string& headers, const std::string& name, std::string& outValue)
{
    size_t start = 0;
    std::string loweredName = toLower(name);
    while (start < headers.size())
    {
        size_t end = headers.find("\r\n", start);
        if (end == std::string::npos)
            break;
        if (end == start)
            break;
        std::string line = headers.substr(start, end - start);
        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string key = toLower(trimSpaces(line.substr(0, colon)));
            if (key == loweredName)
            {
                outValue = trimSpaces(line.substr(colon + 1));
                return (true);
            }
        }
        start = end + 2;
    }
    return (false);
}

/*
    isChunkedBodyComplete:
        check if a chunked body contains its terminating chunk.
*/
bool isChunkedBodyComplete(const std::string& buffer, size_t bodyStart)
{
    size_t idx = bodyStart;
    while (idx < buffer.size())
    {
        size_t lineEnd = buffer.find("\r\n", idx);
        if (lineEnd == std::string::npos)
            return (false);
        std::string sizeStr = buffer.substr(idx, lineEnd - idx);
        size_t semicolon = sizeStr.find(';');
        if (semicolon != std::string::npos)
            sizeStr = sizeStr.substr(0, semicolon);
        if (sizeStr.empty())
            return (false);
        unsigned long size = std::strtoul(sizeStr.c_str(), NULL, 16);
        idx = lineEnd + 2;
        if (idx + size + 2 > buffer.size())
            return (false);
        if (buffer.compare(idx + size, 2, "\r\n") != 0)
            return (false);
        if (size == 0)
            return (true);
        idx += size + 2;
    }
    return (false);
}

}
