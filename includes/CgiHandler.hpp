/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:05:31 by gafreire          #+#    #+#             */
/*   Updated: 2026/04/29 12:55:40 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "bookstore.hpp"
#include "ConfigData.hpp"
#include "HttpRequest.hpp"

class CgiHandler
{
	private:
	
		int _pipeIn[2];
		int _pipeOut[2];
		
		char** createEnv(const std::string &scriptPath, const HttpRequest &req);
		void freeEnv(char** envp);
		void executeChild(const std::string &scriptPath, const HttpRequest &req);
		std::string readParent(pid_t pid, const HttpRequest &req);
	public:
		CgiHandler();
		~CgiHandler();

		std::string executeCgi(const std::string &scriptPath, const HttpRequest &req);
};

#endif
