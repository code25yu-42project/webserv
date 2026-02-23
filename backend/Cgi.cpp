#include "Cgi.hpp"

/* ************************************************************************** */
/* ---------------------------------   OCCF  -------------------------------- */
/* ************************************************************************** */

Cgi::Cgi() {}

Cgi::~Cgi() {}

/* ************************************************************************** */
/* -----------------------------------  CGI  -------------------------------- */
/* ************************************************************************** */

// std::string	readPipe(int pipe_fd) {
//  	char		buffer[1024];
//  	size_t		bytes;
//  	std::string	response;
 		
//  	close(pipe_fd + 1);
//  	while ((bytes = read(pipe_fd, buffer, sizeof(buffer) - 1)) > 0) {
//  		buffer[bytes] = '\0';
//  		response.append(buffer);
//  	}
//  	close(pipe_fd);
//  	return response;
//  }

std::pair<int, int> 	Cgi::executeCgi(const std::string &path, const std::string &script,  const std::vector<std::string> &env) {
	int					pipe_fd[2], status, exit_code;
	pid_t				pid;
	std::string			response;
	std::pair<int, int>	cgi_fd;
	
	cgi_fd = std::make_pair(-1, -1);;
	if (pipe(pipe_fd) == -1) {
		// std::cout << "pipe failed" << std::endl;
		// perror("pipe failed");
		// return (cgi_fd);
		Exception::handleSystemError(errno);
	}
	pid = fork();
	if (pid < 0) {
		// std::cout << "fork failed" << std::endl;
		// perror("fork failed");
		// return (cgi_fd);
		Exception::handleSystemError(errno);
	}
	if (pid == 0) {
		runChildProcess(path, script, env, pipe_fd);
	} else {
		// std::cout << "child process: " << readPipe(pipe_fd[0]) << std::endl;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			exit_code = WEXITSTATUS(status);
			// std::cout << "Child process terminated, exit code: " << exit_code << std::endl;
			if (exit_code == 1) {
				// std::cout << "Error: Child process exited with failure (exit(1))" << std::endl;
				// return (cgi_fd);
				Exception::handleSystemError(errno);
			}
 			return (std::make_pair(pipe_fd[0], pipe_fd[1]));
		} else {
			// std::cout << "Child process terminated abnormally" << std::endl;
		}
	}
	return (cgi_fd);
}

void Cgi::runChildProcess(
	const std::string &path,
	const std::string &script,
	const std::vector<std::string> &env,
	int pipe_fd[2])
{
	std::vector<char*>	argv;
	std::vector<char*>	cgi_env;
	size_t				pos;
	std::string			file, dir;

	if (access(script.c_str(), F_OK) == -1)
		exit(1);
	if (access(script.c_str(), X_OK) == -1)
		exit(1);

	pos = script.find_last_of('/');
	if (pos != std::string::npos)
	{
		dir = script.substr(0, pos);
		file = script.substr(pos + 1);
		if (chdir(dir.c_str()) == -1)
			exit(1);
	}

	close(pipe_fd[0]);
	dup2(pipe_fd[1], STDOUT_FILENO);
	close(pipe_fd[1]);

	/* argv */
	argv.push_back(const_cast<char*>(path.c_str()));
	argv.push_back(const_cast<char*>(file.c_str()));
	argv.push_back(NULL);

	/* envp */
	for (size_t i = 0; i < env.size(); ++i)
		cgi_env.push_back(const_cast<char*>(env[i].c_str()));
	cgi_env.push_back(NULL);

	execve(argv[0], &argv[0], &cgi_env[0]);
	exit(1);
}

// void	Cgi::runChildProcess(const std::string &path, const std::string &script, const std::vector<std::string> &env, int pipe_fd[2]) {
// 	int											idx;
// 	char										*argv[3], *cgi_env[env.size() + 1];
// 	size_t										pos;
// 	std::string									file, dir;
// 	std::vector<std::string>::const_iterator	it;

// 	if (access(script.c_str(), F_OK) == -1) {
// 		// std::cout << "Script file does not exist" << std::endl;
// 		// perror("Script file does not exist");
// 		exit(1);
// 	}
// 	if (access(script.c_str(), X_OK) == -1) {
// 		// std::cout << "Script file is not executable" << std::endl;
// 		// perror("Script file is not executable");
// 		exit(1);
// 	}
// 	pos = script.find_last_of('/');
// 	if (pos != std::string::npos) {
// 		dir = script.substr(0, pos);
// 		file = script.substr(pos + 1);
// 		if (chdir(dir.c_str()) == -1) {
// 			// std::cout << "chdir failed." << std::endl;
// 			exit(1);
// 		}
// 	}
// 	close(pipe_fd[0]);
// 	dup2(pipe_fd[1], STDOUT_FILENO);
// 	close(pipe_fd[1]);
// 	argv[0] = const_cast<char*>(path.c_str());
// 	argv[1] = const_cast<char*>(file.c_str());
// 	argv[2] = NULL;
// 	idx = 0;
// 	for (it = env.begin(); it != env.end(); ++it) {
// 		cgi_env[idx++] = const_cast<char*>(it->c_str());
// 	}
// 	cgi_env[idx] = NULL;
// 	execve(argv[0], argv, cgi_env);
// 	// perror("execve failed");
// 	exit(1);
// }


void addEnvVar(std::vector<std::string> &env, const std::string &key, const std::string &value) {
	env.push_back(key + "=" + value);
}

std::vector<std::string> Cgi::createCgiEnv(HttpMsg &request, std::string script_name, std::string path_info, std::string body, std::string user_id) {
	std::string					protocol, host, port, path;
	std::vector<std::string>	env;
	ServerUtils					utils;
	HttpUtils					http_utils;

	utils.parseUrl(request.getStartlineValue("host"), protocol, host, port, path);
	// CGI 필수 변수
	addEnvVar(env, "AUTH_TYPE", request.getHeaderValue("Authorization"));
	addEnvVar(env, "CONTENT_LENGTH", request.getHeaderValue(http_utils.getHeader(CONTENT_LENGTH)));
	addEnvVar(env, "CONTENT_TYPE", request.getHeaderValue(http_utils.getHeader(CONTENT_TYPE)));
	addEnvVar(env, "GATEWAY_INTERFACE", "CGI/1.1");
	addEnvVar(env, "PATH_INFO", path_info);
	addEnvVar(env, "PATH_TRANSLATED", script_name);
	addEnvVar(env, "REQUEST_METHOD", request.getStartlineValue(http_utils.getStartline(METHOD)));
	addEnvVar(env, "SCRIPT_NAME", script_name);
	addEnvVar(env, "SERVER_NAME", host);
	addEnvVar(env, "SERVER_PORT", port);
	addEnvVar(env, "SERVER_PROTOCOL", request.getStartlineValue(http_utils.getStartline(VERSION)));
	addEnvVar(env, "SERVER_SOFTWARE", "webserv/1.0");
	addEnvVar(env, "QUERY_STRING", "");
	// HTTP 관련 헤더
	addEnvVar(env, "REMOTE_ADDR", request.getHeaderValue("X-Forwarded-For"));
	addEnvVar(env, "REMOTE_PORT", request.getHeaderValue("X-Forwarded-Port"));
	addEnvVar(env, "HTTP_USER_AGENT", request.getHeaderValue("User-Agent"));
	addEnvVar(env, "HTTP_ACCEPT", request.getHeaderValue("Accept"));
	addEnvVar(env, "HTTP_ACCEPT_LANGUAGE", request.getHeaderValue("Accept-Language"));
	addEnvVar(env, "HTTP_ACCEPT_ENCODING", request.getHeaderValue("Accept-Encoding"));
	addEnvVar(env, "HTTP_CONNECTION", request.getHeaderValue(http_utils.getHeader(CONNECTION)));
	addEnvVar(env, "HTTP_COOKIE", request.getHeaderValue(http_utils.getHeader(COOKIE)));
	// 추가 정보
	addEnvVar(env, "BODY", body);
	addEnvVar(env, "USER_ID", user_id);
	// std::cout << "cgi body: " << body << std::endl;
	return env;
}