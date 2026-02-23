#ifndef MODEL_HPP
# define MODEL_HPP

# include <dirent.h>   // DIR, struct dirent, opendir, readdir, closedir
# include <sys/stat.h> // struct stat, stat(), S_ISREG, S_ISDIR

# include "Types.hpp"

class Model {
	public:
		/* OCCF */
					Model();
					~Model ();
		/* CREATE */
		void		createResponseHeaders(HttpMsg &request, HttpMsg &response, std::map<std::string, std::string> headers, std::string status);
		/* AUTOINDEX */
		t_vec_vec	getDirVec(const std::string &dir_path);
		int			openDirectory(const std::string &path, DIR *&dir_ptr);
		void		getFileInfo(struct dirent *file, const std::string &dir_path, t_vec_vec &values);
};

#endif
