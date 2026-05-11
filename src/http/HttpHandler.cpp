/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 13:09:23 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/04 12:20:08 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

/*
	Constructors and Destructors
*/
HttpHandler::HttpHandler()
{

}
HttpHandler::~HttpHandler()
{

}

/*
    injectSessionCookie:
        Injects a Set-Cookie header into a raw HTTP response string
        if a new session was just created.
*/
static std::string injectSessionCookie(const std::string& response, const std::string& newSessionId)
{
	if (newSessionId.empty())
		return (response);
	size_t headerEnd = response.find("\r\n");
	if (headerEnd == std::string::npos)
		return (response);
	std::string result = response;
	result.insert(headerEnd + 2, "Set-Cookie: session_id=" + newSessionId + "; Path=/; Max-Age=3600\r\n");
	return (result);
}

/*
    handleRequest:
        main traffic manager, delegates logic to specialized functions
		depending on the HTTP method.
        After getting a response, always injects Set-Cookie if a new
        session was created.
*/
std::string HttpHandler::handleRequest(HttpRequest& req, const ServerConfig& serverConf, int* cgiPipeFd, int* cgiWriteFd)
{
	std::string cookieHeader = "";
	std::map<std::string, std::string> headers = req.getHeaders();
	if (headers.find("Cookie") != headers.end())
		cookieHeader = headers["Cookie"];
	std::string newSessionId = manageSession(cookieHeader);

    std::string method = req.getMethod();
    std::string uri = req.getUri();

	std::string response;
    const LocationConfig* loc = matchLocation(uri, serverConf);
    if (loc != NULL && loc->redirectCode != 0)
    {
        std::string statusText = (loc->redirectCode == 301) ? "301 Moved Permanently" : "302 Found";
        std::string body = "<html><body><h1>" + statusText + "</h1><p>Redirecting to <a href=\"" + loc->redirectUrl + "\">" + loc->redirectUrl + "</a></p></body></html>";
        std::stringstream redir;
        redir << "HTTP/1.1 " << statusText << "\r\n"
              << "Location: " << loc->redirectUrl << "\r\n"
              << "Content-Type: text/html\r\n"
              << "Content-Length: " << body.length() << "\r\n\r\n"
              << body;
        return (injectSessionCookie(redir.str(), newSessionId));
    }
    if (method == "GET")
        response = handleGet(req, serverConf, uri, cgiPipeFd, cgiWriteFd);
    else if (method == "POST")
        response = handlePost(req, serverConf, uri, cgiPipeFd, cgiWriteFd);
    else if (method == "DELETE")
        response = handleDelete(req, serverConf, uri);
	else
		response = buildErrorResponse(405, &serverConf, NULL);

	return (injectSessionCookie(response, newSessionId));
}
