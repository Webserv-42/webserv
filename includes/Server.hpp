/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:09 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 17:45:28 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigData.hpp"
#include "HttpHandler.hpp"
#include "bookstore.hpp"
#include "Client.hpp"
 
/*
    Server:
        Network management: create sockets, bind/listen,
        manage the poll() event loop.
        Delegates all HTTP logic to HttpHandler.
        This is where the main server loop lives.
*/

class Server 
{
    private:
        std::vector<ServerConfig> _configs;
        std::map<int, int> _cgiPipeToClient;
        std::map<int, int> _cgiWritePipeToClient;
        HttpHandler _httpHandler;
        static volatile sig_atomic_t _stop;
    
        std::vector<pollfd> _fds;
        std::map<int, Client> _clients;
        std::map<int, std::vector<ServerConfig*> > _socketToConfigs;
    
        void acceptClient(int serverFd);
        void handleClient(int clientFd);
        void removeClient(int fd);
        void ReadFromClient(Client& c);
        void ProcessRequest(Client& c);
        void sendResponse(Client& c);
        void handleCgiWrite(int pipeFd);
        void setPollEvents(int fd, short events);
        void removePollFd(int fd);
    
    public:
        Server() {}
        ~Server() {}
    
        void init(const std::vector<ServerConfig>& configs); 
        void initSockets();
        void run();
        void registredCgiFd(int pipeFd, int clientFd);
        void handleCgiResponse(int pipeFd);
        void shutdown();
        static void handleSigint(int);
        void checkTimeouts();
};

#endif