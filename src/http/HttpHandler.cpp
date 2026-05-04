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
    handleRequest:
        main traffic manager, delegates logic to specialized functions
		depending on the HTTP method.
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
	if (method == "GET")
		return (handleGet(req, serverConf, uri, cgiPipeFd, cgiWriteFd));
	else if (method == "POST")
		return (handlePost(req, serverConf, uri, cgiPipeFd, cgiWriteFd));
    else if (method == "DELETE")
        return (handleDelete(req, serverConf, uri));
	else
		response = (buildErrorResponse(405, &serverConf, NULL));

	if(!newSessionId.empty())
	{
		size_t headerEnd = response.find("\r\n");
		if(headerEnd != std::string::npos)
			response.insert(headerEnd + 2, "Set-Cookie: session_id=" + newSessionId + "\r\n");
	}
	return (response);
}
