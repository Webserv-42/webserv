/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alejagom <alejagom@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 14:00:09 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/30 13:11:55 by alejagom         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConfigData.hpp"
#include "HttpHandler.hpp"
// #include "ServerConfig.hpp"
#include "bookstore.hpp"
#include "Client.hpp"
 
//rama de prueba_alejo
/*
    Gestion de red, crear sockets, hacer bind/ listen,
    manejar el event pool con poll().
    Delega toda la logica HTPP a HttpHandler.
    Aqui trabaja " "
*/

/*
    class Server:
        - Tiene que contener la logica de socket(), setsockopt(), bind(), 
            listen(), fcntl()
        - Un bucle infinito donde:
            Se implementar el loop de poll()
            Cuando poll() indique que hay datos listos para leer:
                1. LLamar a recv() para obtener el texto
                2. Pasar el texto al HtppHandler
        - Delegar proceso al HtppHandler pasandole la configuracion del servidor correspondiente
        - Cuando poll() indique que podemos escribir (POLLOUT), enviar la respuesta con send()
*/
class Server 
{
private:
    std::vector<ServerConfig> _configs;
    std::map<int, int> _cgiPipeToClient; // Registro de pipes y clientfd. (UNIR CON SAM).
    HttpHandler _httpHandler;
    static volatile sig_atomic_t _stop;

    
    std::vector<pollfd> _fds; //lista de todos los FDs (servidor + clientes)
    std::map<int, Client> _clients; //guarda estado de cada cliente
    std::map<int, ServerConfig*> _socketToConfig; //múltiples puertos múltiples servidores

    void acceptClient(int serverFd); //crea cliente nuevo
    void handleClient(int clientFd); //recv+ buffer
    void removeClient(int fd); // limpia el buffer guardado
    void ReadFromClient(Client& c);
    void ProcessRequest(Client& c);
    void sendResponse(Client& c);

public:
    Server() {}
    ~Server() {}

    void init(const std::vector<ServerConfig>& configs); 
    void initSockets(); //socket bind listen añade a _fds
    void run(); // event loop → recorrer fds → accept o recv
    void registredCgiFd(int pipeFd, int clientFd); // Sam llama esto para el bonus de los pipes (procesos).
    void handleCgiResponse(int pipeFd); // Metodo que lee el pipe del cliente y mete la respuesta del cliente. (Cookies).
    void shutdown();
    static void handleSigint(int);
    void checkTimeouts();
};

#endif