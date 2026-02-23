#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h> //socket
# include <fcntl.h> //fcntl
# include <arpa/inet.h> //IPPROTO_TCP
# include <unistd.h> //close, fork, exec
# include <sys/wait.h> // wait
# include <sys/stat.h> //stat
# include <sys/types.h> //dir
# include <dirent.h> //dir2
# include <ctime> // localtime, strftime
# include <cstdio>

# include "../config/ServerConfig.hpp"
# include "../config/LocationConfig.hpp"
# include "../checker/SyntaxChecker.hpp"
# include "Kqueue.hpp"
# include "Controller.hpp"

class Server {
	private:
		t_listen_map		m_listen_map;
		ServerConfig		m_serv_config;
		Kqueue 				*m_kqueue;
		HttpUtils			m_http_utils;
		/* SET INFO */
		void			setInfo(t_accept &info);
		/* ADD */
		int				addNewAccept(int listen_fd, t_accept info);
		int				addNewConnect(int listen_fd, int accept_fd, const unsigned int &ip, const unsigned short &port);
		int				setSockOpt(const int& fd, const int &rcv_buf, const int &snd_buf);
		int				setNonBlocking(const int& fd);
		void			setProxyPass(HttpMsg &request, const std::string &proxy_pass, std::string &ip, std::string &port);
		int				connectUpstream(int listen_fd, int accept_fd, const std::string &ip, const std::string &port);
		/* RECV SEND */
		int				recvRequestMsg(const int &from_fd, t_accept &accept);
		int				recvResponseMsg(const int &from_fd, t_accept &accept);
		int 			recvCgiMsg(const int &from_fd, t_accept &accept);
		int				sendRequestMsg(HttpMsg &msg, const int &to_fd);
		int				sendResponseMsg(HttpMsg &msg, const int &to_fd);
		void			sendShutdownMessage(HttpMsg &respone, int accept_fd);
		/* UTILS */
		int				handle_socket_error();
		bool			isValidAcceptFd(int listen_fd, int accept_fd);

	public:
		/* OCCF */
						Server();
		virtual			~Server();
		/* GET SET */
		void 			setServerConfig(ServerConfig serv_config);
		/* FIND */
		int						findListen(int listen_fd);
		int						findListenAccept(int accept_fd);
		int						findListenConnect(int connect_fd);
		t_listen_map::iterator	findListenMap(t_listen_map &listen_map, int &listen_fd);
		t_accept_map::iterator	findAcceptMap(t_accept_map &accept_map, int &listen_fd);
		/* ADD */
		int				addNewListen(const unsigned int &ip, const unsigned short &port);
		/* READ WRITE */
		int				readFromListen(int listen_fd);
		int				readFromAccept(int listen_fd, int accept_fd, SessionManager &session);
		int				readFromConnect(int listen_fd, int connect_fd);
		int				readForCgi(int listen_fd, int accept_fd, int cgi_fd, SessionManager &session);
		void			getSessionForUsersRegister(t_accept &accept_info, SessionManager &session);
		int				writeFromAccept(int listen_fd, int accept_fd);
		int				writeFromConnect(int listen_fd, int connect_fd);
		int				writeForShutdown(int listen_fd, int accep_fd);
	};

#endif
