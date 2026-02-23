#include "Exception.hpp"

void	Exception::handleInvalidFile() {
	throw std::logic_error("invalid config file.");
}

void	Exception::handleInvalidFile(int _errno) {
	std::string lines;

	lines = "invalid config file.\n";
	throw std::logic_error(lines + strerror(_errno));
}

//DEBUG
void	Exception::handleInvalidAccess() {
	throw std::logic_error("it does not have context. check your context name again.");
}

void Exception::handleSystemError(int _errno) {
	throw std::runtime_error(strerror(_errno));
}
