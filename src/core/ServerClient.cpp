/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerClient.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/03 20:10:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/03 20:10:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerClientUtils.hpp"
#include <cctype>

/*
    setPollEvents:
        1. look for the fd in the poll list
        2. update your events
*/
void Server::setPollEvents(int fd, short events)
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = events;
            break;
        }
    }
}

/*
    removePollFd:
        1. look for the fd in the poll list
        2. delete your entry
*/
void Server::removePollFd(int fd)
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}

/*
    handleClient:
        1. route the flow according to the customer's status
        2. performs reading, processing, or sending
*/
void Server::handleClient(int clientfd)
{
    Client& c = _clients[clientfd];

    switch (c.state)
    {
        case READING_HEADERS:
        case READING_BODY:
            ReadFromClient(c);
            if (c.state == PROCESSING)
                ProcessRequest(c);
            break;
        case PROCESSING:
            ProcessRequest(c);
            break;
        case CGI_WAITING:
            break;
        case SENDING:
            sendResponse(c);
            break;
        case DONE:
            removeClient(clientfd);
            break;
        default:
            break;
    }
}

/*
    ReadFromClient:
        1. read data from the socket
        2. detects end of headers and Content-Length
        3. update the status according to the body received
*/
void Server::ReadFromClient(Client& c)
{
    char buffer[4096];
    int n = recv(c.fd, buffer, sizeof(buffer), 0);

    if (n <= 0)
    {
        c.state = DONE;
        return ;
    }
    c.lastActivity = time(NULL);
    c.buffer.append(buffer, n);
    if (c.state == READING_HEADERS)
    {
        size_t posEnd = c.buffer.find("\r\n\r\n");
        if (posEnd != std::string::npos)
        {
            std::string headersBlock = c.buffer.substr(0, posEnd + 2);
            std::string transferEncoding;
            if (ServerClientUtils::getHeaderValue(headersBlock, "Transfer-Encoding", transferEncoding))
            {
                std::string lowered = ServerClientUtils::toLower(transferEncoding);
                if (lowered.find("chunked") != std::string::npos)
                {
                    if (!ServerClientUtils::isChunkedBodyComplete(c.buffer, posEnd + 4))
                    {
                        c.state = READING_BODY;
                        return ;
                    }
                    c.state = PROCESSING;
                    return ;
                }
            }

            std::string contentLength;
            if (ServerClientUtils::getHeaderValue(headersBlock, "Content-Length", contentLength))
            {
                c.ContLength = (size_t)std::atol(contentLength.c_str());
                if (c.ContLength > (size_t)c.config->clientMaxBodySize)
                {
                    c.response = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\n\r\n";
                    c.bytesSend = 0;
                    c.state = SENDING;
                    setPollEvents(c.fd, POLLIN | POLLOUT);
                    return ;
                }
                c.state = READING_BODY;
            }
            else
                c.state = PROCESSING;
        }
    }
    if (c.state == READING_BODY)
    {
        size_t headerEnd = c.buffer.find("\r\n\r\n") + 4;
        std::string headersBlock = c.buffer.substr(0, headerEnd);
        std::string transferEncoding;
        if (ServerClientUtils::getHeaderValue(headersBlock, "Transfer-Encoding", transferEncoding))
        {
            std::string lowered = ServerClientUtils::toLower(transferEncoding);
            if (lowered.find("chunked") != std::string::npos)
            {
                if (ServerClientUtils::isChunkedBodyComplete(c.buffer, headerEnd))
                    c.state = PROCESSING;
                return ;
            }
        }
        size_t bodyRecibed = c.buffer.size() - headerEnd;
        if (bodyRecibed >= c.ContLength)
            c.state = PROCESSING;
    }
}

/*
    ProcessRequest:
        1. parse the HTTP request and detect keep-alive
        2. execute the handler and manage CGI if applicable
*/
void Server::ProcessRequest(Client& c)
{
    std::string cookie = "";
    size_t pos = c.buffer.find("Cookie: ");
    if (pos != std::string::npos)
    {
        size_t end = c.buffer.find("\r\n", pos);
        cookie = c.buffer.substr(pos + 8, end - (pos + 8));
    }
    c.cookie = cookie;
    std::vector<ServerConfig*> configs;
    std::map<int, std::vector<ServerConfig*> >::iterator it = _socketToConfigs.find(c.serverFd);
    if (it != _socketToConfigs.end())
        configs = it->second;
    const ServerConfig *config = c.config;
    if (!configs.empty())
        config = configs[0];

    HttpRequest req;
    req.parse(c.buffer);
    if (config != NULL && !configs.empty())
    {
        std::map<std::string, std::string> headers = req.getHeaders();
        if (headers.find("Host") != headers.end())
        {
            std::string host = headers["Host"];
            size_t colon = host.find(':');
            if (colon != std::string::npos)
                host = host.substr(0, colon);
            for (size_t i = 0; i < configs.size(); i++)
            {
                if (configs[i]->serverName == host)
                {
                    config = configs[i];
                    break;
                }
            }
        }
    }
    if (config == NULL)
    {
        c.state = DONE;
        return ;
    }
    c.config = config;
    c.keepAlive = false;
    std::map<std::string, std::string> headers = req.getHeaders();
    if (headers.find("Connection") != headers.end())
    {
        std::string connStr = headers["Connection"];
        for (size_t i = 0; i < connStr.length(); i++)
            connStr[i] = std::tolower(connStr[i]);
        if (connStr.find("keep-alive") != std::string::npos)
            c.keepAlive = true;
    }

    int cgiPipeFd = -1;
    int cgiWriteFd = -1;
    c.response = _httpHandler.handleRequest(req, *config, &cgiPipeFd, &cgiWriteFd);
    c.bytesSend = 0;
    if (cgiPipeFd != -1)
    {
        registredCgiFd(cgiPipeFd, c.fd);
        c.state = CGI_WAITING;
        if (cgiWriteFd != -1)
        {
            c.cgiWriteFd = cgiWriteFd;
            c.cgiBody = req.getBody();
            c.cgiBodySent = 0;
            if (c.cgiBody.empty())
            {
                close(c.cgiWriteFd);
                c.cgiWriteFd = -1;
            }
            else
            {
                pollfd pdf;
                pdf.fd = c.cgiWriteFd;
                pdf.events = POLLOUT;
                pdf.revents = 0;
                _fds.push_back(pdf);
                _cgiWritePipeToClient[c.cgiWriteFd] = c.fd;
            }
        }
    }
    else
    {
        c.state = SENDING;
        setPollEvents(c.fd, POLLIN | POLLOUT);
    }
}

/*
    sendResponse:
        1. send the response partially
        2. adjust status and keep-alive upon completion
*/
void Server::sendResponse(Client& c)
{
    const char* data = c.response.c_str() + c.bytesSend;
    size_t remaining = c.response.size() - c.bytesSend;
    int n = send(c.fd, data, remaining, 0);

    if (n <= 0)
    {
        c.state = DONE;
        return ;
    }
    c.bytesSend += n;
    if (n > 0)
        c.lastActivity = time(NULL);
    if (c.bytesSend >= c.response.size())
    {
        setPollEvents(c.fd, POLLIN);
        if (c.keepAlive)
        {
            c.buffer.clear();
            c.response.clear();
            c.bytesSend = 0;
            c.ContLength = 0;
            c.keepAlive = false;
            c.state = READING_HEADERS;
        }
        else
            c.state = DONE;
    }
}

/*
    handleCgiWrite:
        1. write the pending body in the CGI pipe
        2. close the pipe when finished.
*/
void Server::handleCgiWrite(int pipeFd)
{
    int clientFd = _cgiWritePipeToClient[pipeFd];
    Client& c = _clients[clientFd];
    if (c.cgiBodySent >= c.cgiBody.size())
    {
        close(pipeFd);
        removePollFd(pipeFd);
        _cgiWritePipeToClient.erase(pipeFd);
        c.cgiWriteFd = -1;
        c.cgiBody.clear();
        c.cgiBodySent = 0;
        return ;
    }
    const char* data = c.cgiBody.c_str() + c.cgiBodySent;
    size_t remaining = c.cgiBody.size() - c.cgiBodySent;
    int n = write(pipeFd, data, remaining);
    if (n <= 0)
    {
        close(pipeFd);
        removePollFd(pipeFd);
        _cgiWritePipeToClient.erase(pipeFd);
        c.cgiWriteFd = -1;
        c.cgiBody.clear();
        c.cgiBodySent = 0;
        return ;
    }
    c.cgiBodySent += n;
    if (c.cgiBodySent >= c.cgiBody.size())
    {
        close(pipeFd);
        removePollFd(pipeFd);
        _cgiWritePipeToClient.erase(pipeFd);
        c.cgiWriteFd = -1;
        c.cgiBody.clear();
        c.cgiBodySent = 0;
    }
}

/*
    removeClient:
        1. close the client socket
        2. clean your state and pollfd
*/
void Server::removeClient(int fd)
{
    Client& c = _clients[fd];
    if (c.cgiWriteFd != -1)
    {
        close(c.cgiWriteFd);
        removePollFd(c.cgiWriteFd);
        _cgiWritePipeToClient.erase(c.cgiWriteFd);
    }
    close(fd);
    _clients.erase(fd);
    removePollFd(fd);
}
