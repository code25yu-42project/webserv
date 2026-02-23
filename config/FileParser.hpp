#ifndef FILEPARSER_HPP
# define FILEPARSER_HPP
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
# include "../exception/Exception.hpp"
# include "AConfig.hpp"

class FileParser {
    private:
		std::deque<std::string>				m_frags;
		size_t								findEndCharsetFromStrs(char end);
		size_t								splitBySpaces(std::string& line, size_t& prev, size_t& end);
		std::string							fileToStr(std::string file);

    public:
        FileParser();
		void								getFile(std::string file);
      
        FileParser(std::string file);
        ~FileParser();
		void	setDefault(AConfig &obj);

};

#endif