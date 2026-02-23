#include "Server.hpp"

const int	QUE_SIZE = 10000;
const int	SNDBUF = 100000;
const int	RCVBUF = 100000;
// const int	KB = 1024;

/* ************************************************************************** */
/* ------------------------------	OCCF	------------------------------- */
/* ************************************************************************** */

/* PUBLIC */
Server::Server() {
	m_listen_map.clear();
	m_serv_config = ServerConfig();
	m_kqueue = Kqueue::getInstance();
	// std::cout << "[Class Server] Server initialized." << std::endl; 
}

Server::~Server() {
	int						connect_fd;
	t_listen_map::iterator	it_listen;
	t_accept_map::iterator	it_accept;

	/* Close Listen fd */
	for (it_listen = m_listen_map.begin(); it_listen != m_listen_map.end(); ++it_listen) {
		/* Close Accept fd */
		for (it_accept = it_listen->second.accept_map.begin();
			it_accept != it_listen->second.accept_map.end(); ++it_accept) {
			/* Close Connect fd */
			if (it_listen->second.connection.accept_key.find(it_accept->first) !=
				it_listen->second.connection.accept_key.end()){
				connect_fd = it_listen->second.connection.accept_key[it_accept->first];
				if (connect_fd > 0)
				close(connect_fd); 
			}
			if (it_accept->first > 0) {
				sendShutdownMessage(it_accept->second.response, it_accept->first);
				// m_kqueue->deleteReadWriteEvent(it_accept->first);
				close(it_accept->first);
				// std::cout << "[Socket] Successfully closed Accept socket fd: " << it_accept->first << std::endl;
			}
		}
		if (it_listen->first > 0) {
			// m_kqueue->deleteReadWriteEvent(it_listen->first);
			close(it_listen->first);
			// std::cout << "[Socket] Successfully closed Listen socket fd: " << it_listen->first << std::endl;
		}
	}
	// std::cout << "[Class Server] Server instance destroyed." << std::endl;
}

/* ************************************************************************** */
/* ------------------------------	GET/SET	------------------------------- */
/* ************************************************************************** */

/* public */
void Server::setServerConfig(ServerConfig serv_config) {
	m_serv_config = serv_config;
}

/* ************************************************************************** */
/* ------------------------------	FIND	------------------------------- */
/* ************************************************************************** */

/* public */
int	Server::findListen(int listen_fd) { // listen_fd가 listen_map에 존재하는지, 반환값: listen_fd
	if (m_listen_map.find(listen_fd) != m_listen_map.end()) {
		return (listen_fd);
	}
	return (-1); 
}

int	Server::findListenAccept(int accept_fd) { // accept_fd가 어떤 listen_fd에 존재하는지, 반환값: listen_fd
	t_listen_map::iterator it;

	for (it = m_listen_map.begin(); it != m_listen_map.end(); ++it) {
		if (it->second.accept_map.find(accept_fd) != it->second.accept_map.end())
			return (it->first);
	}
	return (-1);
}

int	Server::findListenConnect(int connect_fd) { // connect_fd가 어떤 listen_fd에 존재하는지, 반환값: listen_fd 
	t_listen_map::iterator it;

	for (it = m_listen_map.begin(); it != m_listen_map.end(); ++it) {
		if (it->second.connection.connect_key.find(connect_fd) \
		!= it->second.connection.connect_key.end())
			return (it->first);
	}
	return (-1);
}

t_listen_map::iterator	Server::findListenMap(t_listen_map &listen_map, int &listen_fd) {
	t_listen_map::iterator	it_listen;

	for (it_listen = listen_map.begin(); it_listen != listen_map.end(); ++it_listen) {
		if (it_listen->first == listen_fd)
			return (it_listen);
	}
	return (listen_map.end());
}

t_accept_map::iterator	Server::findAcceptMap(t_accept_map &accept_map, int &listen_fd) {
	t_accept_map::iterator	it_accept;

	for (it_accept = accept_map.begin(); it_accept != accept_map.end(); ++it_accept) {
		if (it_accept->first == listen_fd)
			return (it_accept);
	}
	return (accept_map.end());
}

/* ************************************************************************** */
/* ------------------------------	SET INFO  ------------------------------- */
/* ************************************************************************** */

/* private */
void Server::setInfo(t_accept &info) {
	unsigned int	header_size, body_size;

	try {
		header_size = m_serv_config.getter("client_header_buffer_size");
		// std::cout << "header_size:" << header_size << std::endl;
	} catch (std::exception &e) {
		// header_size = 1 * KB;
	}
	try {
		body_size = m_serv_config.getter("client_body_buffer_size");
		// std::cout << "body_size:" << body_size << std::endl;
	} catch (std::exception &e) {
		// body_size = 16 * KB;
	}
	info.header_size = header_size;
	info.body_size = body_size;
	info.proxy_pass = "";
	info.cgi_fd.first = -1;
	info.cgi_fd.second = -1;
	info.request = HttpMsg();
	info.response = HttpMsg();
}

/* ************************************************************************** */
/* ---------------------------------	ADD	------------------------------ */
/* ************************************************************************** */

/* public */
int	Server::addNewListen(const unsigned int &ip, const unsigned short &port) {
	int					listen_fd;
	struct sockaddr_in	addr;

	listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_fd < 0) {
		return (handle_socket_error());
	}
	if (setSockOpt(listen_fd, RCVBUF, SNDBUF) < 0)
		return (handle_socket_error());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip;
	if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
		return (handle_socket_error());
	} 
	if (listen(listen_fd, QUE_SIZE) < 0)  {
		return (handle_socket_error());
	}
	if (setNonBlocking(listen_fd) < 0) {
		// std::cout << "set" << std::endl;
		return (handle_socket_error());
	}
	m_kqueue->addReadEvent(listen_fd, -1, LISTEN);
	m_listen_map[listen_fd];
	ServerUtils utils;
	std::cout << "[add New Listen] New Listen Added: "
			<< "Listen FD = " << findListenMap(m_listen_map, listen_fd)->first \
			<< " | Port = " << ntohs(utils.getSockname(findListenMap(m_listen_map, listen_fd)->first).sin_port)
			<< " | Map Size = " << m_listen_map.size()
			<< std::endl; 
	return (listen_fd);
}

/* private */
int	Server::addNewAccept(int listen_fd, t_accept info) {
	int					accept_fd;
	socklen_t			accept_addr_len;
	struct sockaddr_in	addr;
	ServerConfig		config;

	accept_addr_len = sizeof(addr.sin_addr);
	accept_fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&addr.sin_addr), &accept_addr_len);
	if (accept_fd < 0)
		return (handle_socket_error());
	if (setSockOpt(accept_fd, SO_RCVBUF, SO_SNDBUF) < 0)
		return (handle_socket_error());
	if (setNonBlocking(accept_fd) < 0)
		return (handle_socket_error());
	m_kqueue->addReadEvent(accept_fd, listen_fd, ACCEPT);
	m_listen_map[listen_fd].accept_map[accept_fd] = info;
	ServerUtils utils;
	std::cout << "[add New Accept] New Accept Added: "
			<< "Accept FD = " << findAcceptMap(m_listen_map[listen_fd].accept_map, accept_fd)->first \
			<< " | Port = " << ntohs(utils.getSockname(findAcceptMap(m_listen_map[listen_fd].accept_map, accept_fd)->first).sin_port)
			<< " | Map Size = " << m_listen_map[listen_fd].accept_map.size()
			<< std::endl; 
	return (accept_fd);
}

int	Server::addNewConnect(int listen_fd, int accept_fd, const unsigned int &ip, const unsigned short &port) {
	int					connect_fd;
	struct sockaddr_in	addr;
	t_listen			&listen = m_listen_map[listen_fd];

	connect_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect_fd < 0)
		return (handle_socket_error());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip;
	if (connect(connect_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
		close(connect_fd);
		return (handle_socket_error());
	}
	if (setNonBlocking(connect_fd) < 0)
		return (handle_socket_error());
	m_kqueue->addReadEvent(connect_fd, listen_fd, CONNECT);
	listen.connection.accept_key[accept_fd] = connect_fd;
	listen.connection.connect_key[connect_fd] = accept_fd;
	ServerUtils utils;
	std::cout << "[add New Connect] New Connect Added: "
			<< "Connect FD = " << listen.connection.accept_key[accept_fd] \
			<< " | Port = " << ntohs(utils.getSockname(listen.connection.accept_key[accept_fd]).sin_port)
			<< std::endl;  
	return (connect_fd);
}

/*
	[setsockopt 옵션 정보]
	SO_REUSERADDR : port 재사용
	SO_RCVBUF : 소켓 단위 수신 크기 
	SO_SNDBUF : 소켓 단위 송신 크기 
*/

int Server::setSockOpt(const int &fd, const int &rcv_buf, const int &snd_buf) {
	int	reuse, status;

	reuse = 1;
	status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if (status != -1)
		status = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, sizeof(rcv_buf));
	if (status != -1)
		status = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, sizeof(snd_buf));
	return (status);
}

int Server::setNonBlocking(const int& fd) {
	int	flag;

	flag = fcntl(fd, F_GETFL, 0);
	if (flag != -1) 
		return (fcntl(fd, F_SETFL, flag | O_NONBLOCK));
	return (flag);
}

void	Server::setProxyPass(HttpMsg &request, const std::string &proxy_pass, std::string &ip, std::string &port) {
	std::string		protocol, path;
	ServerUtils 	utils;

	utils.parseUrl(proxy_pass, protocol, ip, port, path);
	request.setStartline(m_http_utils.getStartline(URI), path);
	return ;
}

int	Server::connectUpstream(int listen_fd, int accept_fd, const std::string &ip, const std::string &port) {
	t_listen 		&listen = m_listen_map[listen_fd];
	SyntaxChecker	checker;
	int				connect_fd;

	// std::cout << "IP: " << ip << std::endl; 
	try {
		connect_fd = addNewConnect(listen_fd, accept_fd, checker.makeIP(ip), checker.checkValidNum(port, 65535));
	} catch (std::exception &e) {
		listen.accept_map[accept_fd].response.setStatusCode(m_http_utils.getStatus(INTERNAL_SERVER_ERROR_500));
	}
	if (connect_fd < 0) 
		listen.accept_map[accept_fd].response.setStatusCode(m_http_utils.getStatus(INTERNAL_SERVER_ERROR_500));
	return (connect_fd);
}

/* ************************************************************************** */
/* ------------------------------	READ WRITE	------------------------------- */
/* ************************************************************************** */

/* public */
int	Server::readFromListen(int listen_fd) {
	int			accept_fd;
	t_accept	info;

	setInfo(info);
	accept_fd = addNewAccept(listen_fd, info);
	return (accept_fd);
}

int	Server::readFromAccept(int listen_fd, int accept_fd, SessionManager &session) {
	// int			connect_fd, socket_status;
	int			socket_status;
	std::string	ip, port;
	
	if (isValidAcceptFd(listen_fd, accept_fd) == false) {
		return (SOCKET_CLOSE);
	}
	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];
	// socket_status = recvRequestMsg(500, accept_fd, accept_info);
	socket_status = recvRequestMsg(accept_fd, accept_info);
	/* SOCKET CHECK : CLOSE/OPEN/NEXT */
	if (socket_status == SOCKET_CLOSE){
		m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, SHUTDOWN); // delete read accept (ACCEPT) + add wrtie accept (SHUTDOWN)
		// std::cout << "SOCKET CLOSE" << std::endl;
		return (socket_status);
	} else if (socket_status == SOCKET_OPEN) { // timeout 추가 
		// std::cout << "SOCKET OPEN" << std::endl;
		return (socket_status);
	}
	/* SOCKET NEXT */
	Controller	controller(accept_info, m_serv_config, session);
	controller.createResponse();
	if (!accept_info.proxy_pass.empty()) { // 프록시 패스가 있을 경우
		// std::cout << "[readFromAccept] need connect" << std::endl;
		setProxyPass(accept_info.request, accept_info.proxy_pass, ip, port);
		// // connect_fd = connectUpstream(listen_fd, accept_fd, ip, port);
		// accept_info.proxy_pass = true;
		if (connectUpstream(listen_fd, accept_fd, ip, port) < 0) {
			accept_info.proxy_pass.clear();
			accept_info.response.setStatusCode(m_http_utils.getStatus(INTERNAL_SERVER_ERROR_500));
		}
		m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, ACCEPT); // delete read accept (ACCEPT) + add write accept (ACCEPT)
	} else { // 프록시 패스가 없을 경우
		// std::cout << "[readFromAccept] no need connect" << std::endl;
		if (accept_info.cgi_fd.first != -1) {
			// std::cout << "[readFromAccept] need CGI" << std::endl;
			// m_kqueue->deleteReadEvent(accept_fd);
			m_kqueue->deleteaddReadEvent(accept_fd, accept_info.cgi_fd.first, listen_fd, accept_fd, CGI);
		} 
		else 
			m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, ACCEPT);  // delete read accept (ACCEPT) + add write accept (ACCEPT)
	}
	// std::cout << "SOCKET NEXT" << std::endl;
	return (socket_status);
}

int Server::readFromConnect(int listen_fd, int connect_fd) {
	int	accept_fd, socket_status;

	accept_fd = m_listen_map[listen_fd].connection.connect_key[connect_fd];
	// socket_status = recvResponseMsg(500, accept_fd, m_listen_map[listen_fd].accept_map[accept_fd]);	
	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];
	socket_status = recvResponseMsg(connect_fd, accept_info);
	if (socket_status == SOCKET_CLOSE) {
		m_kqueue->deleteaddWriteEvent(connect_fd, connect_fd, listen_fd, SHUTDOWN);
		// std::cout << "SOCKET CLOSE" << std::endl;
		return (socket_status);
	}
	if (socket_status == SOCKET_OPEN) {
		// std::cout << "SOCKET OPEN" << std::endl;
		return (socket_status);
	}
	m_kqueue->deleteReadaddWriteEvent(connect_fd, connect_fd, listen_fd, CONNECT); 
	// std::cout << "SOCKET NEXT" << std::endl;
	return (socket_status);
}

int Server::readForCgi(int listen_fd, int accept_fd, int cgi_fd, SessionManager &session) {
	int			socket_status;
	std::string	ip, port;

	// std::cout << "Read For CGI" << std::endl;
	if (isValidAcceptFd(listen_fd, accept_fd) == false) {
		return (SOCKET_CLOSE);
	}
	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];
	socket_status = recvCgiMsg(cgi_fd, accept_info);
	if (socket_status == SOCKET_CLOSE) {
		m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, SHUTDOWN); // delete read accept (ACCEPT) + add wrtie accept (SHUTDOWN)
		// std::cout << "SOCKET CLOSE" << std::endl;
		return (socket_status);
	} else if (socket_status == SOCKET_OPEN) { // timeout 추가 
		// std::cout << "SOCKET OPEN" << std::endl;
		return (socket_status);
	}
	getSessionForUsersRegister(accept_info, session);
	m_kqueue->addWriteEvent(accept_fd, listen_fd, ACCEPT);
	close(accept_info.cgi_fd.first);
	close(accept_info.cgi_fd.second);
	// m_kqueue->deleteaddWriteEvent(cgi_fd, accept_fd, listen_fd, ACCEPT);
	// std::cout << "CGI END" << std::endl;
	return (socket_status);
}

void	Server::getSessionForUsersRegister(t_accept &accept_info, SessionManager &session) {
	t_session_data	data;
	std::string		cookie, sessionid;

	if (!accept_info.response.getBody().find("error")) 
		return ;
	cookie = accept_info.request.getHeaderValue(m_http_utils.getHeader(COOKIE)); // 세션 아이디가 요청에 포함되어 있는지 확인
	sessionid = session.extractSessionId(cookie);
	if (sessionid.empty() || session.isValidSessionID(sessionid) == false) {
		accept_info.sessionid = session.createSession(data);
		accept_info.response.addHeader(m_http_utils.getHeader(SET_COOKIE), "sessionid=" + accept_info.sessionid + "; Max-Age=3600; Path=/; HttpOnly" );
		session.setSessionData(accept_info.sessionid, accept_info.response.getBody());
	} else
		accept_info.sessionid = sessionid;
	// session.printSessionMap();
	// std::cout << "CGI: " << accept_info.sessionid << std::endl;
}

int Server::writeFromAccept(int listen_fd, int accept_fd) {
	int			connect_fd, socket_status;
	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];

	if (accept_info.proxy_pass.empty()) {
		// std::cout << "TO accept_fd FROM accept_fd" << std::endl;
		// socket_status = sendResponseMsg(accept_info.response, 500);
		socket_status = sendResponseMsg(accept_info.response, accept_fd);
		/* SOCKET CHECK : CLOSE/OPEN/NEXT */
		if (socket_status == SOCKET_CLOSE) {
			m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, SHUTDOWN); // delete write accept (ACCEPT) + add write accept_fd (SHUTDOWN)
			// std::cout << "SOCKET CLOSE" << std::endl;
			return (socket_status);
		} else if (socket_status == SOCKET_OPEN) {
			// std::cout << "SOCKET OPEN" << std::endl;
			return (socket_status);
		}
		/* SOCKET NEXT */
		/* PROXY_PASS = FALSE 마무리 */ 
		m_listen_map[listen_fd].accept_map.erase(accept_fd);
		m_kqueue->deleteWriteEvent(accept_fd);
		close(accept_fd);
	} else {
		// std::cout << "TO connect_fd FROM accept_fd" << std::endl;
		connect_fd = m_listen_map[listen_fd].connection.accept_key[accept_fd];
		// socket_status = sendRequestMsg(accept_info.request, 500);
		socket_status = sendRequestMsg(accept_info.request, connect_fd);
		/* SOCKET CHECK : CLOSE/OPEN/NEXT */
		if (socket_status == SOCKET_CLOSE) {
			// accept_info.shutdown = true;
			m_kqueue->deleteReadaddWriteEvent(accept_fd, accept_fd, listen_fd, SHUTDOWN); // delete write accept (ACCEPT) + add write accept_fd (SHUTDOWN)
			// std::cout << "SOCKET CLOSE" << std::endl;
			return (socket_status);
		} else if (socket_status == SOCKET_OPEN) {
			// std::cout << "SOCKET OPEN" << std::endl;
			return (socket_status);
		}
		/* SOCKET NEXT */
		m_kqueue->deleteWriteAddReadEvent(accept_fd, connect_fd, listen_fd, CONNECT);
	}
	// std::cout << "SOCKET NEXT" << std::endl; 
	return (socket_status);
}

int Server::writeFromConnect(int listen_fd, int connect_fd) {
	int	accept_fd, socket_status;
	
	// std::cout << "write From Connect " << std::endl;
	
	accept_fd = m_listen_map[listen_fd].connection.connect_key[connect_fd];
	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];
	// socket_status = sendResponseMsg(m_listen_map[listen_fd].accept_map[accept_fd].response, 500);
	socket_status = sendResponseMsg(accept_info.response, accept_fd);
	if (socket_status == SOCKET_CLOSE) {
		m_kqueue->deleteaddWriteEvent(connect_fd, connect_fd, listen_fd, SHUTDOWN); 	// delete write connect (CONNECT) + add write connect_fd (SHUTDOWN)
		// std::cout << "SOCKET CLOSE" << std::endl;
		return (socket_status);
	}
	else if (socket_status == SOCKET_OPEN) {
		// std::cout << "SOCKET OPEN" << std::endl;
		return (socket_status);
	}
	/* PROXY_PASS = TRUE 마무리 */ 
	m_listen_map[listen_fd].connection.connect_key.erase(connect_fd);
	m_listen_map[listen_fd].connection.accept_key.erase(accept_fd);
	m_listen_map[listen_fd].accept_map.erase(accept_fd);
	m_kqueue->deleteWriteEvent(connect_fd);
	close(connect_fd);
	close(accept_fd);
	// std::cout << "SOCKET NEXT" << std::endl;
	return (socket_status);
}

int	Server::writeForShutdown(int listen_fd, int accept_fd) {
	int connect_fd;
	// std::cout << "SHUTDOWN" << std::endl;

	t_accept	&accept_info = m_listen_map[listen_fd].accept_map[accept_fd];
	sendShutdownMessage(accept_info.response, accept_fd);
	m_listen_map[listen_fd].accept_map.erase(accept_fd);
	m_kqueue->deleteWriteEvent(accept_fd);
	close(accept_fd);
	if (m_listen_map[listen_fd].connection.accept_key.find(accept_fd) \
		!= m_listen_map[listen_fd].connection.accept_key.end())	{
		connect_fd = m_listen_map[listen_fd].connection.accept_key[accept_fd];
		m_listen_map[listen_fd].connection.accept_key.erase(accept_fd);
		m_listen_map[listen_fd].connection.connect_key.erase(connect_fd);
		m_kqueue->deleteWriteEvent(connect_fd);
		close(connect_fd);
	}
	return (SOCKET_CLOSE);
}

/* ************************************************************************** */
/* ------------------------------	RECV SEND	------------------------------- */
/* ************************************************************************** */

/* private */
int Server::recvCgiMsg(const int &from_fd, t_accept &accept) {
	char				buffer[1024];
	int					bytes_lines;
	std::string			msg;
	std::stringstream	ss_length;
	HttpMsg				&response = accept.response;

	bytes_lines = read(from_fd, buffer, sizeof(buffer) - 1);
	if (bytes_lines < 0) {
		// std::cout << "Recv error: Could not read Data." << std::endl;
	} else if (bytes_lines == 0) {
		// std::cout << "EOF reached on pipe_fd[0]" << std::endl;
	} else {
		buffer[bytes_lines] = '\0';
		// std::cout << "Received CGI response: " << buffer << std::endl;
	}
	msg = buffer;
	// ServerUtils utils;
	// utils.printSendMsg(bytes_lines, from_fd);
	// bytes_lines = response.recvBuffer(from_fd, msg, accept.header_size + accept.body_size);
	// if (bytes_lines <= 0)
	// 	return (SOCKET_CLOSE);
	if (msg.find("error"))
		response.setBody(msg);
	ss_length << msg.size();
	accept.response.addHeader(m_http_utils.getHeader(CONTENT_LENGTH), ss_length.str());
	return (SOCKET_NEXT);
}

int	Server::recvRequestMsg(const int &from_fd, t_accept &accept) {
	int			bytes_lines;
	std::string	msg, http_status;
	HttpMsg		&request = accept.request;

	// bytes_lines = request.recvBuffer(from_fd, msg, 0);
	bytes_lines = request.recvBuffer(from_fd, msg, accept.header_size + accept.body_size);
	if (bytes_lines <= 0)
		return (SOCKET_CLOSE);
	// ServerUtils utils;
	// utils.printSendMsg(bytes_lines, from_fd);
	request.setHttpMsg(msg);
	// std::cout << "[FULL REQUEST]" << std::endl << request.getHttpMsg() << std::endl;
	request.setRecvType(request.checkHttpType(request.getHttpMsg()));
	// std::cout << "request.getRecvType(): " << request.getRecvType() << std::endl;
	if (request.getRecvType() == BODY_COMPLETE) {
		// std::cout << "ACCEPT BODY SIZE" << accept.body_size << std::endl;
		http_status = request.isValidRequest(request.getHttpMsg(), accept.header_size, accept.body_size);
		if (http_status == m_http_utils.getStatus(OK_200))
			request.parseMsg(request.getHttpMsg(), REQUEST); 
		else 
			request.parseStartline(request.getHttpMsg(), REQUEST);
		if (accept.response.getStatusCode().empty() \
		|| accept.response.getStatusCode() == m_http_utils.getStatus(OK_200))
			accept.response.setStatusCode(http_status);
		// std::cout << "[FULL REQUEST]" << std::endl << request.getHttpMsg()<< std::endl;
	} else {
		return (SOCKET_OPEN);
	}
	return (SOCKET_NEXT);
}

int Server::recvResponseMsg(const int &from_fd, t_accept &accept) {
	int			bytes_lines;
	std::string	msg, http_status;
	HttpMsg		&response = accept.response;

	// bytes_lines = response.recvBuffer(from_fd, msg, 0);
	bytes_lines = response.recvBuffer(from_fd, msg, accept.header_size + accept.body_size);
	if (bytes_lines <= 0)
		return (SOCKET_CLOSE);
	// ServerUtils utils;
	// utils.printSendMsg(bytes_lines, from_fd);
	response.setHttpMsg(msg);
	response.setRecvType(response.checkHttpType(response.getHttpMsg()));
	// std::cout << "response.getRecvType(): " << response.getRecvType() << std::endl;
	if (response.getRecvType() == BODY_COMPLETE) {
		http_status = response.isValidResponse(response.getHttpMsg(), accept.header_size, accept.body_size);
		response.parseMsg(response.getHttpMsg(), RESPONSE);
		if (response.getStatusCode().empty() \
		|| response.getStatusCode() == m_http_utils.getStatus(OK_200))
			response.setStatusCode(http_status);
	} else {
		return (SOCKET_OPEN);
	}
	return (SOCKET_NEXT);
}

// 요청 메시지 전송 함수
int Server::sendRequestMsg(HttpMsg &http_msg, const int &to_fd) {
	int			bytes_sent, size, total_sent; 
	std::string	merged_msg, remaining_msg;

	total_sent = 0;
	// 현재까지 전송된 크기 출력
	// std::cout << "[sendRequestMsg] Sent Size: " << http_msg.getSentSize() << " bytes" << std::endl;
	// HTTP 메시지 병합
	merged_msg = http_msg.mergeHttpMsg(REQUEST);
	size = http_msg.getSentSize();
	// 전송할 메시지 부분 구하기
	remaining_msg = merged_msg.substr(size); 
	// 전송할 크기 출력
	// std::cout << "[sendRequestMsg] Sending: " << merged_msg.size() - size << " bytes" << std::endl;
	// 실제 전송
	bytes_sent = http_msg.sendBuffer(to_fd, remaining_msg, total_sent);
	if (bytes_sent <= 0) {
		// 전송 실패시
		http_msg.addSentSize(total_sent);
		// std::cout << "[sendRequestMsg] Transmission failed. Closing socket." << std::endl;
		return SOCKET_CLOSE;
	} else {
		// 전송 성공시
		http_msg.addSentSize(total_sent);
		// ServerUtils utils;
		// utils.printSendMsg(bytes_sent, to_fd);
		// 전송 후 상태 출력
		// std::cout << "[sendRequestMsg] Sent Size: " << http_msg.getSentSize() << " bytes" << std::endl;
		// std::cout << "[sendRequestMsg] Total Message Size: " << merged_msg.size() << " bytes" << std::endl;
		// 아직 전송할 데이터가 남았으면
		if (http_msg.getSentSize() < static_cast<int>(merged_msg.size())) {
			// std::cout << "[sendRequestMsg] Partial message sent. Returning SOCKET_OPEN." << std::endl;
			return SOCKET_OPEN;
		}
	}
	// 모든 메시지가 전송된 후
	// std::cout << "[sendRequestMsg] Message fully sent. Returning SOCKET_NEXT." << std::endl;
	return SOCKET_NEXT;
}

// 응답 메시지 전송 함수
int Server::sendResponseMsg(HttpMsg &http_msg, const int &to_fd) {
	int			bytes_sent, size, total_sent;
	std::string	merged_msg, remaining_msg;

	total_sent = 0;
	// 현재까지 전송된 크기 출력
	// std::cout << "[sendResponseMsg] Sent Size: " << http_msg.getSentSize() << " bytes" << std::endl;
	// HTTP 메시지 병합
	// std::cout << http_msg.getHttpMsg() << std::endl;
	merged_msg = http_msg.mergeHttpMsg(RESPONSE);
	size = http_msg.getSentSize();
	// 전송할 메시지 부분 구하기
	remaining_msg = merged_msg.substr(size); 
	// 전송할 크기 출력
	// std::cout << "[sendResponseMsg] Sending: " << merged_msg.size() - size << " bytes" << std::endl;
	// 실제 전송
	bytes_sent = http_msg.sendBuffer(to_fd, remaining_msg, total_sent);
	if (bytes_sent <= 0) {
		// 전송 실패시
		http_msg.addSentSize(total_sent);
		// std::cout << "[sendResponseMsg] Transmission failed. Closing socket." << std::endl;
		return SOCKET_CLOSE; // 클라이언트 닫아야 함
	} else {
		// 전송 성공시
		http_msg.addSentSize(total_sent);
		// ServerUtils utils;
		// utils.printSendMsg(bytes_sent, to_fd);
		// 전송 후 상태 출력
		// std::cout << "[sendResponseMsg] Sent Size: " << http_msg.getSentSize() << " bytes" << std::endl;
		// std::cout << "[sendResponseMsg] Total Message Size: " << merged_msg.size() << " bytes" << std::endl;
		// 아직 전송할 데이터가 남았으면
		if (http_msg.getSentSize() < static_cast<int>(merged_msg.size())) {
			// std::cout << "[sendResponseMsg] Partial message sent. Returning SOCKET_OPEN." << std::endl;
			return SOCKET_OPEN;  // 연결 지속
		}
	}
	// 모든 메시지가 전송된 후
	// std::cout << "[sendResponseMsg] Message fully sent. Returning SOCKET_NEXT." << std::endl;
	return SOCKET_NEXT;
}

void Server::sendShutdownMessage(HttpMsg &response, int accept_fd) {
	std::string	status, err_page_url;
	View		view;
	
	response.setStartline(m_http_utils.getStartline(VERSION), "HTTP/1.1");
	status = m_http_utils.getStatus(INTERNAL_SERVER_ERROR_500);
	response.setStartline(m_http_utils.getStartline(STATUS_CODE), status);
	response.setStartline(m_http_utils.getStartline(STATUS_MESSAGE), status);
	response.setHeader(m_http_utils.getHeader(CONNECTION), "close");
	err_page_url = m_serv_config.getErrorPage(status);
	response.setRedirect(view.redirectToJson(err_page_url));
	// response.setHeader(m_http_utils.getHeader(LOCATION), "");
	sendResponseMsg(response, accept_fd);
}

/* ************************************************************************** */
/* ------------------------------------  UTILS	----------------------------- */
/* ************************************************************************** */

/* pirvate */
int Server::handle_socket_error() {
	if (errno == EINTR) { // 신호에 의해 accept가 중단된 경우
		// std::cout << "Accept was interrupted by signal, retrying..." << std::endl;
	} else if (errno == EMFILE) { // 프로세스가 열 수 있는 파일 디스크립터 수가 초과된 경우
		// std::cout << "Too many open files, try again later" << std::endl;
	} else if (errno == ENFILE) { // 시스템 전체에서 열 수 있는 파일 디스크립터 수가 초과된 경우
		// std::cout << "System-wide file descriptor limit reached" << std::endl;
	} else {
		// perror("Socket failed"); // 기타 다른 오류
	}
	return (-1);
}

bool	Server::isValidAcceptFd(int listen_fd, int accept_fd) {
	// std::cout << "[isValidAcceptFd] Validation started..." << std::endl;
	// m_listen_map이 비어있는 경우
	if (m_listen_map.empty()) {
		// std::cout << "[isValidAcceptFd] Error: m_listen_map is empty!" << std::endl;
		return false;
	}
	// listen_fd가 m_listen_map에 존재하는지 확인
	if (m_listen_map.find(listen_fd) == m_listen_map.end()) {
		// std::cout << "[isValidAcceptFd] Error: listen_fd not found in m_listen_map!" << std::endl;
		return false;
	}
	// accept_map이 비어있는지 확인
	t_accept_map &accept_map = m_listen_map[listen_fd].accept_map;
	if (accept_map.empty()) {
		// std::cout << "[isValidAcceptFd] Error: accept_map is empty for listen_fd!" << std::endl;
		return false;
	}
	// accept_fd가 accept_map에 존재하는지 확인
	if (accept_map.find(accept_fd) == accept_map.end()) {
		// std::cout << "[isValidAcceptFd] Error: accept_fd not found in accept_map!" << std::endl;
		return false;
	}
	// accept_fd가 accept_map에 존재하는 경우
	// std::cout << "[isValidAcceptFd] Success: accept_fd found in accept_map!" << std::endl;
	return true;
}