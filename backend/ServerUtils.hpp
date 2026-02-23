#ifndef SERVERUTILS_HPP
# define SERVERUTILS_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <vector>
# include <map>
# include <sys/socket.h> //socket
# include <netinet/in.h> //sockaddr_in
# include <arpa/inet.h>

class ServerUtils {
	public:
		/* OCCF */
					ServerUtils();
					~ServerUtils();
		/* string */
		std::string	findStringContaining(const std::vector<std::string>& vec, const std::string& target);
		std::string	extractSubstring(const std::string& input, std::string start, std::string end);
		/* print */
		void		printMap(std::map<std::string, std::string>& my_map);
		void		printVector(const std::vector<int> &vec);
		void		printVector(const std::vector<std::string> &vec);
		void		printSendMsg(int bytes_sent, int fd);
		void		printRecvMsg(int bytes_sent, int fd);
		/* parse */
		void		parseUrl(const std::string &url, std::string &protocol, std::string &host, std::string &port, std::string &path);
		/* socket */
		sockaddr_in	getSockname(int socket_fd);
		void		printSockInfo(int socket_fd);
		/* file */
		std::string	getFileExtension(const std::string& path);
		std::string	getFileToStr(const std::string &file_path);
		bool		saveStringToFile(const std::string &file_path, const std::string &content);
		void		clearFile(const std::string &filename);

};

#endif