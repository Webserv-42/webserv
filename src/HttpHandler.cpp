/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 13:09:23 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/28 10:24:08 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

// Constructores y Destructores vacíos
HttpHandler::HttpHandler() 
{
    
}
HttpHandler::~HttpHandler() 
{
    
}

/*
    getMimeType:
        funcion para que detecte el tipo de mime css,jpg...
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
    handleRequest:
        1. Buscamos el mejor bloque Location para la URI que nos han pedido
        - NGINX concatena el Root al URI completo (ej: root "/var/www" + "/images/gato.jpg")
        - Pero le quitamos los dobles '/' por si acaso se solapan.
        
*/
std::string HttpHandler::handleRequest(HttpRequest& req, const ServerConfig& serverConf) 
{
    (void)serverConf;
    std::string method;
    std::string uri;
        
    method = req.getMethod();
    uri = req.getUri();
    std::cout << "[HTTP HANDLER] Procesando petición: " << method << " " << uri << std::endl;
    
    if (method == "GET") 
    {
        std::string filePath;
        const LocationConfig* loc = matchLocation(uri, serverConf);
        
        if (loc != NULL) 
        {
            if (!loc->allowedMethods.empty()) 
            {
                bool isAllowed = false;
                for (size_t i = 0; i < loc->allowedMethods.size(); i++) 
                {
                    if (loc->allowedMethods[i] == method) 
                    {
                        isAllowed = true;
                        break;
                    }
                }
                if (!isAllowed)
                    return (buildErrorResponse(405));
            }
            filePath = loc->root + uri;
            
            struct stat s;
            if (stat(filePath.c_str(), &s) == 0) 
            {
                if (s.st_mode & S_IFDIR) 
                {
                    bool indexServed = false;
                    if (loc->index != "") 
                    {
                        std::string indexFilePath = filePath;
                        if (indexFilePath[indexFilePath.length() - 1] != '/')
                            indexFilePath += "/";
                        indexFilePath += loc->index;

                        struct stat sIdx;
                        // Si el index existe físicamente, simulamos pedirlo
                        if (stat(indexFilePath.c_str(), &sIdx) == 0 && !(sIdx.st_mode & S_IFDIR)) 
                        {
                            filePath = indexFilePath;
                            indexServed = true;
                        }
                    }

                    // Si el índex no saltó, listamos la carpeta o expulsamos
                    if (!indexServed) 
                    {
                        if (loc->autoindex) {
                            return (generateDirectoryListing(filePath, uri));
                        } else {
                            return (buildErrorResponse(403));
                        }
                    }
                }
            }
        } 
        else 
        {
            filePath = "www" + uri;
            if (uri == "/")
                filePath = "www/index.html";
        }
        std::ifstream file(filePath.c_str());
        if (!file.is_open()) 
            return (buildErrorResponse(404));
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();        
        std::string contentType = getMimeType(filePath);    
        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: " << contentType << "\r\n"
                 << "Content-Length: " << content.length() << "\r\n"
                 << "\r\n"
                 << content;
        return (response.str());
    } 
    else if (method == "POST") 
    {
        // 1. Buscamos Localizador en la Config
        const LocationConfig* loc = matchLocation(uri, serverConf);
        
        // 2. Si no existe configuración o "Upload" no está encendido: Denegado.
        if (loc == NULL || loc->upload_enable == false) {
            return buildErrorResponse(403); 
        }
        // 3. Destino autorizado
        std::string uploadDir = loc->upload_store;
        
        // Para no usar time(), aprovechamos un contador 'static' que conservará su 
        // valor en C++ durante toda la vida del servidor, cumpliendo el Subject al 100%.
        static int fileCounter = 0;
        
        std::stringstream urlBuilder; 
        urlBuilder << uploadDir << (uploadDir[uploadDir.length() - 1] == '/' ? "" : "/") << "uploaded_" << fileCounter << ".bin";
        std::string fullPath = urlBuilder.str();
        
        fileCounter++; // El próximo será file_1, file_2, etc..
        // 4. Escribimos la información recibida usando std::ofstream (Librería estándar de C++ puramente permitida)
        std::ofstream outFile(fullPath.c_str(), std::ios::binary);
        if (!outFile.is_open()) {
            return buildErrorResponse(500); // 500 Internal Error (ej: carpeta no existe)
        }
        
        outFile << req.getBody(); // Escribimos el paquete al disco duro.
        outFile.close();
        // 5. ¡Subida exitosa y respuesta 201 Created! (Código HTTP de "Recurso Creado")
        std::string resBody = "El archivo se ha subido correctamente al servidor Webserv!\n";
        std::stringstream response;
        response << "HTTP/1.1 201 Created\r\n"
                 << "Location: " << fullPath << "\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "Content-Length: " << resBody.length() << "\r\n"
                 << "\r\n"
                 << resBody;
                 
        return response.str();
    }
    else if (method == "DELETE") 
    {
        // 1. Buscamos Localizador en la Config
        const LocationConfig* loc = matchLocation(uri, serverConf);
        
        // 2. Si la ruta pertenece a una configuración, validamos que DELETE esté en la lista permitida
        if (loc != NULL && !loc->allowedMethods.empty()) 
        {
            bool isAllowed = false;
            for (size_t i = 0; i < loc->allowedMethods.size(); i++) {
                if (loc->allowedMethods[i] == method) isAllowed = true;
            }
            if (!isAllowed) return (buildErrorResponse(405)); // No permitido
        }
        // 3. Montamos la ruta del disco duro real.
        // Si hay una config (loc), usamos el directorio "root". Si no la hay (un acceso desprotegido normal) usamos "www" por defecto.
        std::string filePath;
        if (loc != NULL) {
            filePath = loc->root + uri;
        } else {
            filePath = "www" + uri;
        }
        // 4. ¡LA HORA DE LA VERDAD! Llamamos a "remove" que viene en <cstdio> o <unistd.h>
        // Funciona borrando un archivo del mac/linux basándose en su string puramente en C.
        if (remove(filePath.c_str()) == 0) 
        {
            // Código 0 significa que se ha borrado limpiamente.
            // HTTP responde a esto normalmente con un sobrio "204 No Content" (que significa "Hecho, todo borrado, no tengo nada más que añadir").
            std::stringstream response;
            response << "HTTP/1.1 204 No Content\r\n"
                     << "\r\n";
            return response.str();
        } 
        else 
        {
            // Si retorna distinto de 0 puede ser que el archivo no existiera de base o por permisos del sistema operativo.
            return buildErrorResponse(404);
        }
    }
    return (buildErrorResponse(405));
}

/*
    matchLocation:
        Recorremos todos los "Location" del servidor
        Si lo que pide el usuario empieza por locPath (ej: pide /images/gato.jpg y locPath es /images/)
        Nos devolverá la configuración adecuada o NULL si ninguna coincide
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
    buildErrorResponse:
        1. Escogemos el mensaje según tu código
        2. Montamos el esqueleto HTML
        3. Montamos la respuesta HTTP final con el header Content-Type correcto!
*/
std::string HttpHandler::buildErrorResponse(int statusCode) 
{
    std::string statusText;
    std::string message;

    if (statusCode == 404) 
    {
        statusText = "404 Not Found";
        message = "Oops! El archivo que buscas no ha sido encontrado en el servidor.";
    } 
    else if (statusCode == 405) 
    {
        statusText = "405 Method Not Allowed";
        message = "El metodo HTTP utilizado no esta permitido en esta ruta.";
    } 
    else 
    {
        statusText = "500 Internal Server Error";
        message = "Un error inesperado desato el caos dentro del servidor.";
        statusCode = 500;
    }

    // mover este html a un .html!!!!!
    std::stringstream html;
    html << "<!DOCTYPE html>\n<html>\n"
         << "<head><title>" << statusText << "</title></head>\n"
         << "<body style=\"font-family: Arial, sans-serif; text-align: center; margin-top: 10%; background-color: #f4f4f4;\">\n"
         << "   <h1 style=\"font-size: 5rem; color: #ff4a4a; margin-bottom: 0;\">" << statusCode << "</h1>\n"
         << "   <h2 style=\"color: #333;\">" << statusText.substr(4) << "</h2>\n" // Cortamos los 3 números iniciales
         << "   <p style=\"color: #666; margin-bottom: 30px;\">" << message << "</p>\n"
         << "   <hr style=\"width: 50%;\">\n"
         << "   <p style=\"color: #aaa; font-size: 0.8rem;\">Webserv / Proyecto 42</p>\n"
         << "</body>\n</html>";

    std::string htmlBody = html.str();

    std::stringstream response;
    response << "HTTP/1.1 " << statusText << "\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << htmlBody.length() << "\r\n"
             << "\r\n"
             << htmlBody;

    return (response.str());
}

/*
    generateDirectoryListing:
        Abre una carpeta física y genera un HTML enlazado
        con todos los nombres de los archivos.
        1. Intentamos abrir el directorio a bajo nivel
        2. Cabeceras visuales de nuestro HTML (HTML Mover a un .html !!!!)
        3. Iteramos todos los chismes que haya en el directorio
            Saltamos el enlace estresante al subdirectorio "hacia la nada" '.'
            Añadimos "/" al final si es una carpeta real
            ¡Creamos el link dinámico!
        4. Cerramos y limpiamos
        5. ¡A empaquetar de vuelta al cliente! 
*/
std::string HttpHandler::generateDirectoryListing(const std::string& physicalPath, const std::string& currentUri) 
{
    DIR* dir;
    struct dirent* entry;
    std::stringstream html;

    dir = opendir(physicalPath.c_str());
    if (dir == NULL)
        return buildErrorResponse(403); 

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
        html << "  <li style=\"margin: 5px 0;\"><a href=\"" << currentUri << (currentUri[currentUri.length()-1] == '/' ? "" : "/") << displayName << "\">" 
             << displayName << "</a></li>\n";
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

    return response.str();
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
