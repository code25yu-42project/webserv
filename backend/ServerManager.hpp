#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <csignal>
# include "../config/AppConfig.hpp"
# include "Server.hpp"

typedef enum e_stop_flag {
	STOP_SERVER_MANAGER,
	RUN_SERVER_MANAGER
} t_stop_flag;

class ServerManager {
	private:
		static volatile sig_atomic_t	m_stop_flag;
		int								m_kq;
		std::vector<Server>				m_server;    // idx: server_id;
		SessionManager 					m_session;  
	public:
		/* OCCF */
					ServerManager(std::string config_path);
					~ServerManager();
		/* RUN */
		void		runServer();
		/* EVENT */
		void		handleEvent();
		void		handleReadEvent(const int &server_id, const int &listen_fd, const int &now_fd, const t_event_type &sock_type);
		void		handleWriteEvent(const int &server_id, const int &listen_fd, const int &now_fd, const t_event_type &sock_type);
		/* FIND */
		int			findServerId(const int &now_fd, int &listen_fd);
		int			findServerIdByListenFd(const int &listen_fd);
		int			findServerIdByAcceptFd(const int &accept_fd, int &listen_fd);
		int			findServerIdByConnectFd(const int &connect_fd, int &listen_fd);
		/* SIGNAL */
		static void	signalHandler(int signum);
		void		setupSignalHandler();
		

};

#endif