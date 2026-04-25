#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "bookstore.hpp"
#include "ConfigData.hpp"

class CgiHandler
{
	public:
		CgiHandler();
		~CgiHandler();

		std::string executeCgi(const std::string &scriptPath, const std::string &body);

	private:

};

#endif
