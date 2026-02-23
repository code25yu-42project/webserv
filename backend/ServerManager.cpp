#include "ServerManager.hpp"

const int KEVENT_CNT = 100;
volatile sig_atomic_t ServerManager::m_stop_flag = RUN_SERVER_MANAGER;

/* ************************************************************************** */
/* -----------------------------	 OCCF	 ------------------------------- */
/* ************************************************************************** */

ServerManager::ServerManager(std::string config_path) \
: m_kq(Kqueue::getInstance()->getKqueue()) {
	std::vector<unsigned int>	ips;
	std::vector<unsigned short>	ports;
	size_t						listen_nums, idx;
	int							listen_fd;
	bool						run_server;
	AppConfig					*app_config;
	std::vector<ServerConfig>	serv_configs;

	m_server.clear();
	m_session = SessionManager();
	app_config = AppConfig::getInstance(config_path);
	ips = app_config->getIPs();
	ports = app_config->getPorts();
	serv_configs = app_config->getServerConfigs();
	listen_nums = serv_configs.size();
	m_server.resize(listen_nums);
	run_server = false;
	for (idx = 0; idx < listen_nums; ++idx) {
		listen_fd = m_server[idx].addNewListen(ips[idx], ports[idx]);
		if (listen_fd != -1) {
			run_server = true; 
			m_server[idx].setServerConfig(serv_configs[idx]);
		}
	}
	if (run_server == false)
		Exception::handleSystemError(errno);
	setupSignalHandler();
	// std::cout << "[Class ServerManager] ServerManager initialized." << std::endl; 
}

ServerManager::~ServerManager() {
	ServerUtils	utils;

	utils.clearFile(USER_PATH);
	utils.clearFile(CONTENT_PATH);
	utils.clearFile(CHATROOM_PATH);
	// std::cout << "[Class ServerManager] ServerManager instance destroyed." << std::endl;
}

/* ************************************************************************** */
/* ------------------------------	 RUN	 ------------------------------- */
/* ************************************************************************** */

void	ServerManager::runServer() {
	size_t	cycle;

	cycle = -1;
	while (m_stop_flag == RUN_SERVER_MANAGER) {
		std::cout << "=============== Event Cycle " << ++cycle << " ===============" << std::endl;
		// sleep(2);
		handleEvent();
		std::cout << "==============================================" << std::endl;
	}
	// std::cout << "Server shutting down..." << std::endl;
}

/* ************************************************************************** */
/* --------------------------------   EVENT   ------------------------------- */
/* ************************************************************************** */

void printUdata(struct kevent *ev) {
	t_udata *udata = (t_udata *)ev->udata;
	std::cout << "[Udata] EVENT_TYPE: " << udata->event_type 
	<< "| Listen FD : " << udata->listen_fd 
	<< "| Accept FD : " << udata->accept_fd << std::endl;
}

// timeout으로 꺼진 소켓의 데이터는 어떻게 지우지????
void	ServerManager::handleEvent() {
	int				active_nums, idx, now_fd, server_id, listen_fd, accept_fd;
	struct kevent	events[KEVENT_CNT];
	t_udata			*udata;

	listen_fd = -1;
	active_nums = kevent(m_kq, NULL, 0, events, KEVENT_CNT, NULL);
	// std::cout << "[Events Info] Active events: " << active_nums	<< " | Kqueue: " << m_kq << std::endl;
	for (idx = 0; idx < active_nums; ++idx) {
		// ident: 이벤트가 발생한 대상의 파일 디스크립터 | filter: EVFILT_READ -1 EVFILT_WRITE -2 
		std::cout << "[Event idx " << idx << "] Now FD: " << events[idx].ident 
				<< " | Filter: " << events[idx].filter 
				<< " | Udata : " <<  static_cast<t_udata*>(events[idx].udata) << std::endl;
		now_fd = events[idx].ident;
		server_id = findServerId(now_fd, listen_fd);
		listen_fd = -1;
		// std::cout << "[Server Info] Server ID: " << server_id  << " | Listen FD: " << listen_fd  << " | Now FD: " << now_fd << std::endl;
		if (events[idx].udata) {
			udata = static_cast<t_udata*>(events[idx].udata);
			listen_fd = udata->listen_fd;
			accept_fd = udata->accept_fd;
			// printUdata(&events[idx]);
		}
		if (events[idx].filter == EVFILT_READ) {
			if (udata->event_type == CGI) {
				server_id = findServerId(accept_fd, listen_fd);
				m_server[server_id].readForCgi(listen_fd, accept_fd, now_fd, m_session);
			} else 
				handleReadEvent(server_id, listen_fd, now_fd, udata->event_type);
		} else if (events[idx].filter == EVFILT_WRITE) {
			handleWriteEvent(server_id, listen_fd, now_fd, udata->event_type);
		} else {
			// std::cout << "[Event " << idx << "] Unknown event filter: " << events[idx].filter << std::endl;
		}
	}
}

void ServerManager::handleReadEvent(const int &server_id, const int &listen_fd, const int &now_fd, const t_event_type &event_type) {
	std::cout << "[Event] Read event detected." << std::endl;
	if (event_type == LISTEN) {
		std::cout << "[Server] Handling LISTEN socket "
		<< now_fd << " read..." << std::endl;
		m_server[server_id].readFromListen(now_fd); // 에러시 timeout에 의해 소켓 종료 예정
	} else if (event_type == ACCEPT) { // 메세지가 요청일 경우: now_fd accept의 fd 
		std::cout << "[Server] Handling ACCEPT socket "
		<< now_fd << " read..." << std::endl;
		m_server[server_id].readFromAccept(listen_fd, now_fd, m_session); // 에러시 timeout에 의해 소켓 종료 예정
	} else if (event_type == CONNECT) { // 메세지가 응답일 경우: now_fd == connect의 fd
		std::cout << "[Server] Handling CONNECT socket "
		<< now_fd << " read..." << std::endl;
		m_server[server_id].readFromConnect(listen_fd, now_fd);
	}
}

void ServerManager::handleWriteEvent(const int &server_id, const int &listen_fd, const int &now_fd, const t_event_type&event_type) {
	std::cout << "[Event] Write event detected." << std::endl;
	if (event_type == SHUTDOWN) { // SHUTDOWN 처리
		m_server[server_id].writeForShutdown(listen_fd, now_fd);
	} else if (event_type== ACCEPT) {
		std::cout << "[Server] Handling ACCEPT socket "
		<< now_fd << " write..." << std::endl;
		m_server[server_id].writeFromAccept(listen_fd, now_fd);
	} else if (event_type == CONNECT) {
		std::cout << "[Server] Handling CONNECT socket "
		<< now_fd << " write..." << std::endl;
		m_server[server_id].writeFromConnect(listen_fd, now_fd);
	} 
}

/* ************************************************************************** */
/* ------------------------------	 FIND	------------------------------- */
/* ************************************************************************** */

int	ServerManager::findServerId(const int &now_fd, int &listen_fd) {
	int	server_id;

	server_id = findServerIdByListenFd(now_fd);
	if (server_id < 0)
		server_id = findServerIdByAcceptFd(now_fd, listen_fd);
	if (server_id < 0)
		server_id = findServerIdByConnectFd(now_fd, listen_fd);
	return (server_id);
}

int	ServerManager::findServerIdByListenFd(const int &listen_fd) {
	std::vector<Server>::iterator	it;
	int								server_id;

	it = m_server.begin();
	for (server_id = 0; it != m_server.end(); ++server_id, ++it) {
		if (m_server[server_id].findListen(listen_fd) != -1){
			return (server_id); 
		}
	}
	return (-1);
}

int ServerManager::findServerIdByAcceptFd(const int &accept_fd, int &listen_fd) {
	std::vector<Server>::iterator	it;
	int								server_id;

	it = m_server.begin();
	for (server_id = 0; it != m_server.end(); ++server_id, ++it) {
		// std::cout << "server_id - " << server_id << std::endl;
		listen_fd = m_server[server_id].findListenAccept(accept_fd);
		// std::cout << "??" << listen_fd << std::endl;
		if (listen_fd >= 0) {
			return (server_id); 
		}	
	}
	return (-1);
}

int ServerManager::findServerIdByConnectFd(const int &connect_fd, int &listen_fd) {
	std::vector<Server>::iterator	it;
	int								server_id;

	it = m_server.begin();
	for (server_id = 0; it != m_server.end(); ++server_id, ++it) {
		listen_fd = m_server[server_id].findListenConnect(connect_fd);
		if (listen_fd >= 0)
			return (server_id); 
	}
	return (-1);
}

/* ************************************************************************** */
/* ------------------------------	SIGNAL   ------------------------------- */
/* ************************************************************************** */

void ServerManager::signalHandler(int signum) {
	(void)signum;
	// std::cout << "\n[Class ServerManager] Received signal " << signum << ". Stopping server..." << std::endl;
	m_stop_flag = STOP_SERVER_MANAGER;
}

void ServerManager::setupSignalHandler() {
	signal(SIGINT, signalHandler);  // Ctrl+C (SIGINT) 핸들링
	signal(SIGTERM, signalHandler); // 종료 요청 (SIGTERM) 핸들링
	signal(SIGTSTP, signalHandler); // Ctrl+Z (SIGTSTP) 핸들링
	signal(SIGPIPE, SIG_IGN);
}
