#ifndef CHECKER_HPP
# define CHECKER_HPP
# include <iostream>
# include <fstream>
# include <string>
# include <sys/stat.h>
# include <dirent.h>
#include <vector>
# include "../exception/Exception.hpp"

class Checker {
    public:
        static bool	        isExecutableDirectory(std::string path);
        static bool         isDirectory(const std::string &path);
        static bool         isFileExist(std::string path);
        static bool         isOneofFilesExist(const std::string &root, std::string &path, const std::vector<std::string> &files);
        static std::string  makePath(const std::string &path);
    private:

};
# endif