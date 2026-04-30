/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 13:09:23 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/29 13:27:47 by gafreire         ###   ########.fr       */
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

std::string HttpHandler::manageSession(const std::string &cookieHeader)
{
	size_t pos = cookieHeader.find("session_id=");
	if(pos != std::string::npos)
	{
		std::string sessionId = cookieHeader.substr(pos + 11);
		if(_sessionManager.isValidSession(sessionId))
			return "";
	}
	return _sessionManager.createSession();
}

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
        Director de tráfico principal. Delega la lógica en funciones especializadas
        dependiendo del método HTTP.
*/
std::string HttpHandler::handleRequest(HttpRequest& req, const ServerConfig& serverConf)
{
	std::string cookieHeader = "";
	std::map<std::string, std::string> headers = req.getHeaders();
	if (headers.find("Cookie") != headers.end())
		cookieHeader = headers["Cookie"];
	std::string newSessionId = manageSession(cookieHeader);

    std::string method = req.getMethod();
    std::string uri = req.getUri();

    std::cout << "[HTTP HANDLER] Procesando petición: " << method << " " << uri << std::endl;

	std::string response;
    if (method == "GET")
        response = (handleGet(req, serverConf, uri));
    else if (method == "POST")
        response = (handlePost(req, serverConf, uri));
    else if (method == "DELETE")
        response = (handleDelete(req, serverConf, uri));
	else
		response = (buildErrorResponse(405, &serverConf, NULL));

	if(!newSessionId.empty())
	{
		size_t headerEnd = response.find("\r\n");
		if(headerEnd != std::string::npos)
			response.insert(headerEnd + 2, "Set-Cookie: session_id=" + newSessionId + "\r\n");
	}
	return response;
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
std::string HttpHandler::buildErrorResponse(int statusCode, const ServerConfig* serverConf, const LocationConfig* loc)
{
    std::string customContent = "";
    if (loc != NULL) {
        std::map<int, std::string>::const_iterator it = loc->errorPages.find(statusCode);
        if (it != loc->errorPages.end()) {
            customContent = getErrorPageContent(statusCode, *loc);
        }
    } else if (serverConf != NULL) {
        std::map<int, std::string>::const_iterator it = serverConf->errorPages.find(statusCode);
        if (it != serverConf->errorPages.end()) {
            std::string fullPath = it->second;
            if (!fullPath.empty() && fullPath[0] == '/') {
                std::ifstream file(fullPath.substr(1).c_str());
                if (file) {
                    std::ostringstream buffer;
                    buffer << file.rdbuf();
                    customContent = buffer.str();
                } else {
                    std::ifstream file2(fullPath.c_str());
                    if (file2) {
                        std::ostringstream buffer;
                        buffer << file2.rdbuf();
                        customContent = buffer.str();
                    }
                }
            } else {
                std::ifstream file(fullPath.c_str());
                if (file) {
                    std::ostringstream buffer;
                    buffer << file.rdbuf();
                    customContent = buffer.str();
                }
            }
        }
    }

    std::string statusText;
    std::string message;

    if (statusCode == 400) {
        statusText = "400 Bad Request";
        message = "El servidor no pudo entender la solicitud.";
    } else if (statusCode == 403) {
        statusText = "403 Forbidden";
        message = "Acceso denegado a este recurso.";
    } else if (statusCode == 404) {
        statusText = "404 Not Found";
        message = "Oops! El archivo que buscas no ha sido encontrado en el servidor.";
    } else if (statusCode == 405) {
        statusText = "405 Method Not Allowed";
        message = "El metodo HTTP utilizado no esta permitido en esta ruta.";
    } else if (statusCode == 413) {
        statusText = "413 Payload Too Large";
        message = "El cuerpo de la solicitud es demasiado grande.";
    } else if (statusCode == 501) {
        statusText = "501 Not Implemented";
        message = "El servidor no soporta la funcionalidad requerida.";
    } else {
        statusText = "500 Internal Server Error";
        message = "Un error inesperado desato el caos dentro del servidor.";
        statusCode = 500;
    }

    if (!customContent.empty()) {
        std::stringstream response;
        response << "HTTP/1.1 " << statusText << "\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << customContent.length() << "\r\n"
                 << "\r\n"
                 << customContent;
        return (response.str());
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
}
