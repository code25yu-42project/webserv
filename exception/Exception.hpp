#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP
#include <stdexcept>
#include <string>
#include <cerrno>

class Exception 
{
	public:
		static void handleInvalidFile();
		static void handleInvalidFile(int _errno);
		static void	handleInvalidAccess();
		static void handleSystemError(int _errno);
};

#endif
