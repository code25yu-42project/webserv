#include "ServerUtils.hpp"

/* ************************************************************************** */
/* --------------------------------	 OCCF	--------------------------------- */
/* ************************************************************************** */

ServerUtils::ServerUtils() {}

ServerUtils::~ServerUtils() {}

/* ************************************************************************** */
/* --------------------------------	 string	--------------------------------- */
/* ************************************************************************** */

/* puiblic */
std::string	ServerUtils::findStringContaining(const std::vector<std::string>& vec, const std::string& target) {
	size_t	idx;

	for (idx = 0; idx < vec.size(); ++idx) {
		if (vec[idx].find(target) != std::string::npos) {
			return (vec[idx]); 
		}
	}
	return (""); 
}

std::string	ServerUtils::extractSubstring(const std::string& input, std::string start, std::string end) {
	size_t					start_pos, end_pos;
	bool					is_end;
	std::string::iterator	it;

	start_pos = input.find(start);
	if (start_pos == std::string::npos) {
		return ("");
	}
	start_pos += start.size(); 
	is_end = false;
	for (it = end.begin(); it != end.end(); ++it) {
		std::cout << *it << std::endl;
		end_pos = input.find(*it, start_pos);
		if (end_pos != std::string::npos) {
			is_end = true;
			break ;
		}
	}
	if (is_end == false)
		end_pos = input.size();
	return (input.substr(start_pos, end_pos - start_pos)); 
}

bool	ServerUtils::saveStringToFile(const std::string &file_path, const std::string &content) {
	std::ofstream	outfile;
	
	outfile.open(file_path.c_str());
	if (!outfile)
		return (false);
	outfile << content;
	outfile.close();
	return (true);
}

/* ************************************************************************** */
/* --------------------------------	 print	--------------------------------- */
/* ************************************************************************** */

/* puiblic */
void	ServerUtils::printMap(std::map<std::string, std::string>& my_map) {
	std::map<std::string, std::string>::iterator it;

	for (it = my_map.begin(); it != my_map.end(); ++it) {
		std::cout << it->first << " => " << it->second << std::endl;
	}
}

void	ServerUtils::printVector(const std::vector<int> &vec) {
	for (std::vector<int>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;
}

void	ServerUtils::printVector(const std::vector<std::string> &vec) {
	for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); it++) {
		std::cout << *it << std::endl;
	}
	std::cout << std::endl;
}

void	ServerUtils::printSendMsg(int bytes_sent, int fd) {
	if (bytes_sent == -1) {
		std::cout <<"ERROR" << " bytes to now " << fd << std::endl;
	} else {
		std::cout << "Sent " << bytes_sent << " bytes to now to " << fd << std::endl;
	}
}

void	ServerUtils::printRecvMsg(int bytes_sent, int fd) {
	if (bytes_sent == -1) {
		std::cout <<"ERROR" << " bytes from now " << fd << std::endl;
	} else {
		std::cout << "Recv " << bytes_sent << " bytes from now to " << fd << std::endl;
	}
}

/* ************************************************************************** */
/* --------------------------------	 socket	--------------------------------- */
/* ************************************************************************** */

/* puiblic */
sockaddr_in	ServerUtils::getSockname(int socket_fd) {
	sockaddr_in	addr;
	socklen_t	len;

	len = sizeof(addr);
	getsockname(socket_fd, (struct sockaddr *)&addr, &len);
	return (addr);
}

void	ServerUtils::printSockInfo(int socket_fd) {
	sockaddr_in addr;
	socklen_t	len;

	len = sizeof(addr);
	getsockname(socket_fd, (struct sockaddr *)&addr, &len);
	std::cout << "Fd " << socket_fd << " Info: " 
	<< "Port = " << ntohs(addr.sin_port) 
	<< "| IP = " << addr.sin_addr.s_addr << std::endl << std::endl;
}

/* ************************************************************************** */
/* --------------------------------	 parse	--------------------------------- */
/* ************************************************************************** */

/* puiblic */
void	ServerUtils::parseUrl(const std::string &url, std::string &protocol, std::string &host, std::string &port, std::string &path) {
	std::string	temp;
	size_t		pos;

	// 프로토콜 추출
	temp = url;
	pos = temp.find("://");
	if (pos != std::string::npos) {
		protocol = temp.substr(0, pos + 3); // "http:://" 포함
		temp = temp.substr(pos + 3); // "://" 이후 부분
	} else {
		protocol = "http:://"; // 기본값
	}
	// 호스트와 포트 분리
	pos = temp.find(':');
	if (pos != std::string::npos) {
		host = temp.substr(0, pos);
		temp = temp.substr(pos + 1); // ":" 이후 부분
		// 포트 번호 찾기
		pos = temp.find('/');
		if (pos != std::string::npos) {
			port = temp.substr(0, pos);
			path = temp.substr(pos); // '/' 포함한 나머지 경로
		} else {
			port = temp;
			path = "/";
		}
	} else {
		host = temp;
		port = "80"; // 기본값
		path = "/";
	}
}

/* ************************************************************************** */
/* --------------------------------	 file	--------------------------------- */
/* ************************************************************************** */

/* puiblic */
std::string	ServerUtils::getFileExtension(const std::string& path) {
	size_t pos;
	
	pos = path.rfind(".");
	if (pos == std::string::npos || pos == path.size() - 1)
		return ("");
	return (path.substr(pos + 1));
}

std::string	ServerUtils::getFileToStr(const std::string &file_path) {
	std::string			line;
	std::ifstream		file;
	std::stringstream	sstream;

	file.open(file_path);
	if (!file.is_open())
		return ("");
	while (std::getline(file, line))
		sstream << line;
	return (sstream.str());
}

void ServerUtils::clearFile(const std::string &filename) {
	std::ofstream file(filename.c_str(), std::ios::trunc); // 파일 내용을 지움
	if (!file) {
		std::cout << "can't open" << filename << std::endl;
	}
}
