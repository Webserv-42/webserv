/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:05:31 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/01 16:21:34 by gafreire         ###   ########.fr       */
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
		void executeChild(const std::string &executablePath, const std::string &scriptPath, 
			const HttpRequest &req);
	public:
		CgiHandler();
		~CgiHandler();

		int executeCgi(const std::string &executablePath, const std::string &scriptPath, 
			const HttpRequest &req);
};

#endif
