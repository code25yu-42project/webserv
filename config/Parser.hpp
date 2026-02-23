#ifndef PARSER_HPP
# define PARSER_HPP
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <vector>
# include <map>
# include <dirent.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include "ServerConfig.hpp"
# include "LocationConfig.hpp"
// # include "../backend/Format.hpp"
# include "../exception/Exception.hpp"
# include "../checker/SyntaxChecker.hpp"

class AConfig;

class Parser {
	private:
		std::deque<std::string>				m_frags;
		void								bringKeyAndValue(std::string& key, std::string& val, size_t pos);
		std::deque<std::string>				handleWildcard(std::string file);
		void								getFile(std::string file);
		std::string							fileToStr(std::string file);
		void								getFiles(std::string& val);
		void								getFiles(std::deque<std::string> &frag);
		bool								isLocationScope(AConfig &obj) const;
		void								setFragments(std::deque<std::string> &frags, std::string &line);
		size_t								splitByDelimeters(std::deque<std::string> &frags, std::string& line, size_t& prev, size_t& end);
		size_t								splitBySpaces(std::deque<std::string> &frags, std::string& line, size_t& prev, size_t& end);
		void								handleBlockDirectives(AConfig& obj);
		void								handleTypeDirectives(AConfig& obj);
		void								handleSimpleDirectives(AConfig& obj);
		size_t								findEndCharsetFromStrs(char end);
		void								preCheckBlock(AConfig& obj);
		void								postCheckBlock();
		size_t								findValueDeque(const std::string& val) const;

	public:
		Parser(std::string file);
		~Parser();
		void								handleDirectives(AConfig& obj);
		// void								handleRequestForm(Format& format);
		void								printFragments() const;
		std::map<std::string, std::string>	getMap();

};

#include "AConfig.hpp"
#endif