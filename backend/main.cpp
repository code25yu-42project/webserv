#include <iostream>
#include <stdexcept>
#include "ServerManager.hpp"

int	main(int argc, char *argv[]) {
	try {
		if (argc > 2)
			Exception::handleInvalidAccess();
		if (argc == 2) {
			ServerManager	server_manager((std::string(argv[1])));
			server_manager.runServer();
		}
		else {
			ServerManager	server_manager("utils/conf.d/webserv.conf");
			server_manager.runServer();
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return (0);
}