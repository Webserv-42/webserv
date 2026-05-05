#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <string>
#include <map>
#include <cstdlib>
#include <sstream>

/*
	SessionManager:
		simple in-memory session storage that issues session IDs
		and validates them
*/
class SessionManager
{
	private:
		std::map<std::string, std::string> _sessions;

	public:
		SessionManager();
		~SessionManager();

		std::string createSession();
		bool isValidSession(const std::string& sessionId);
};

#endif
