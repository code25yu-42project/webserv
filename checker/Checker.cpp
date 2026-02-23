#include "Checker.hpp"

bool Checker::isOneofFilesExist(const std::string &root, std::string &path, const std::vector<std::string> &files) {
    std::vector<std::string>::const_iterator	cit;
	std::string			path_to_find;
	DIR*				dir;
	
	path = root + path;
  	if ((dir = opendir(path.c_str())) == NULL) {
		return true;
	}
	closedir(dir);

	for (cit = files.begin(); cit != files.end(); cit++)
	{
		path_to_find = path + *cit;
		if (isFileExist(path_to_find))
		{
			path = path_to_find;
			return true;
		}
	}
	return false;
}

bool Checker::isFileExist(std::string path) {
    DIR*			dir;
    struct dirent*	entry;
	std::string		root;
	size_t			pos, npos;
	bool			flag;
	// char			ptr[200];

	pos = 0;
	if (path[0] != '/')
		path.insert(0, "./");
	flag = true;
	while (flag && (npos = path.find('/', pos)) != size_t(-1) && path.size() - 1 != npos)
	{
		flag = false;
		if (npos == 0)
			root = "/";
		else
			root = path.substr(0, npos);
    	if ((dir = opendir(root.c_str())) == NULL) {
        	std::cerr << "Error opening directory: " << root << std::endl;
        	break ;
    	}
	    while (!flag && (entry = readdir(dir)) != NULL) {
    	    if (entry->d_name == path.substr(npos + 1, path.find('/', npos + 1) - npos - 1))
				flag = true;
    	}
	    closedir(dir);
		pos = npos + 1;
	}
	return flag;
}

bool	Checker::isExecutableDirectory(std::string path) {
	return (isFileExist(path));
}
