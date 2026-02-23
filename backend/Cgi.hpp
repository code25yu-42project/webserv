#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>    // std::cout  
# include <unistd.h>    // pipe(), fork(), access(), chdir(), close(), dup2(), execve()  
# include <sys/wait.h>  // waitpid(), WIFEXITED(), WEXITSTATUS()  
# include <vector>      // std::vector  

# include "../exception/Exception.hpp" 
# include "Types.hpp" 

// # define PYTHON_PATH "/usr/local/bin/python"
// # define PHP_PATH "/Users/yubin/.brew/bin/php"
# define PYTHON_PATH "/usr/bin/python3"
# define PHP_PATH "/opt/homebrew/bin/php"
# define USER_PATH "www/cgi-bin/data/users.json"
# define CONTENT_PATH "www/cgi-bin/data/contents.json"
# define CHATROOM_PATH "www/cgi-bin/data/chatroom.json"

class Cgi {
	private:
		void						runChildProcess(const std::string &path, const std::string &script, const std::vector<std::string> &env, int pipe_fd[2]);
	public:
		/* OCCF */
									Cgi();
									~Cgi();
		/* CGI */
		std::pair<int, int> 		executeCgi(const std::string &path, const std::string &script, const std::vector<std::string> &env);
		std::vector<std::string>	createCgiEnv(HttpMsg &request, std::string script_name, std::string path_info, std::string body, std::string user_id);
};

#endif 