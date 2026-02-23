#include "SessionManager.hpp"

const int SESSION_TIMEOUT = 60 * 1;
const std::string SESSIONID_KEY = "sessionid=";

/* ************************************************************************** */
/* --------------------------------  OCCF   --------------------------------- */
/* ************************************************************************** */

/* public */
SessionManager::SessionManager() {
	sessions.clear();
	std::srand(std::time(0));
}

SessionManager::~SessionManager() {}

/* ************************************************************************** */
/* ------------------------------    METHOD   ------------------------------- */
/* ************************************************************************** */

/* public */
std::string	SessionManager::generateNewSessionID() {
	const std::string	charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string			sessionid;
	size_t				idx, size;
	
	size = charset.size();
	while (sessionid.empty() || sessions.find(sessionid) != sessions.end()) {
		for (idx = 0; idx < size; ++idx)
			sessionid += charset[rand() % charset.size()];
	}
	return (sessionid);
}

std::string	SessionManager::createSession(const t_session_data &data) {
	std::string sessionid;

	// 새로운 세션 ID 생성
	sessionid = generateNewSessionID();
	// 세션 데이터 초기화
	sessions[sessionid].data = data;
	sessions[sessionid].creation_time = std::time(0);
	sessions[sessionid].expiration_time = sessions[sessionid].creation_time + SESSION_TIMEOUT;
	// 세션 생성 완료 메시지 출력
	// std::cout << "[createSession]" << std::endl 
	// 		<< "	Session ID: " << sessionid << std::endl
	// 		<< "	Creation Time: " << sessions[sessionid].creation_time << std::endl
	// 		<< "	Expiration Time: " << sessions[sessionid].expiration_time << std::endl
	//  		<< "[createSession] Session created successfully." << std::endl;
	return (sessionid);
}

bool	SessionManager::isValidSessionID(const std::string &sessionid) {
	t_session_map::iterator it;

	// std::cout << "valid session id" << std::endl;
	for (it = sessions.begin(); it != sessions.end(); ++it) {
		// std::cout << "first:" << it->first  << "#" <<std::endl;
		// std::cout << "second:" << sessionid << "#" <<std::endl;
		if (it->first == sessionid)
			return (true);
	}
	return (false);
}

std::string	SessionManager::parseSessionID(const std::string &cookie_header_value) {
	size_t		begin, end;
	std::string	sessionid;
	t_session_map::iterator	it; 
	t_session_data	data;

	begin = cookie_header_value.find(SESSIONID_KEY);
	end = 0;
	if (begin != std::string::npos) {
		begin += 11;
		end = cookie_header_value.find(";", begin);
		if (end == std::string::npos) 
			end = cookie_header_value.size();
	}
	sessionid = cookie_header_value.substr(begin, end - begin);
	// cookie 헤더로 들어온 session ID가 없거나, sessions데이터에 존재하지 않는 session ID일 경우 새로 생성 
	// if (sessionid.empty() || sessions.find(sessionid) == sessions.end()) {
	// 	sessionid = createSession("User Data");
	// }
	// timeout 일 경우 삭제후 다시 생성
	// if (deleteExpiredSession(sessionid) == true)
	// 	sessionid = createSession("User Data");
	return (sessionid);
}

bool	SessionManager::deleteExpiredSession(const std::string &sessionid) {
	t_session_map::iterator it;

	if (std::time(0) > sessions[sessionid].expiration_time) {
		it = sessions.find(sessionid);
		sessions.erase(it);
		return true;
	}
	return false;
}

std::string	SessionManager::manageExpiredSession() {
	t_session_map::iterator it;
	std::string				user_data;

	for (it = sessions.begin(); it != sessions.end(); ++it) {
		user_data = getSessionData(it->first).user_data;
		if (deleteExpiredSession(it->first) == true) {
			// std::cout << "USER DATA " << user_data << std::endl;
			return (user_data);
		}
	}
	return ("");
}

void	SessionManager::updateSessionTimeout(const std::string &sessionid) {
	sessions[sessionid].expiration_time = std::time(0) + SESSION_TIMEOUT;
}

t_session_data	SessionManager::getSessionData(const std::string &sessionid) {
	t_session_data			data;
	// if (deleteExpiredSession(sessionid) == true)
	// 	return ("");
	if (isValidSessionID(sessionid))
		data = sessions[sessionid].data;
	return (data);
	// updateSessionTimeout(sessionid);
}

void	SessionManager::setSessionData(const std::string &sessionid, const std::string &data) {
	t_session_map::iterator	it;

	it = sessions.find(sessionid);
	if (it != sessions.end())
		sessions[sessionid].data.user_data = data;
}

void	SessionManager::printSessionMap() {
	std::cout << "-------------------SessionMap----------------------" << std::endl;
	// map의 모든 항목을 출력
	for (t_session_map::const_iterator it = sessions.begin(); it != sessions.end(); ++it) {
		std::cout << "Session ID: " << it->first << std::endl;

		const t_session &session = it->second;
		
		// data 출력 (여기서는 user_data 출력 예시)
		std::cout << "  User Data: " << session.data.user_data << "#" << std::endl;
		// creation_time과 expiration_time을 human-readable 형태로 출력
		std::cout << "  Creation Time: " << std::ctime(&session.creation_time);
		std::cout << "  Expiration Time: " << std::ctime(&session.expiration_time);
	}
	std::cout << "------------------------------------------------" << std::endl;
}

std::string	SessionManager::extractSessionId(const std::string& cookie) {
	size_t		start_pos, end_pos;
	std::string	sessionid, key;

	key = SESSIONID_KEY;
	start_pos = cookie.find(key);
	if (start_pos != std::string::npos) {
		start_pos += key.length();
		end_pos = cookie.find(";", start_pos);
		if (end_pos == std::string::npos) {
			sessionid = cookie.substr(start_pos);
		} else {
			sessionid = cookie.substr(start_pos, end_pos - start_pos);
		}
		while (!sessionid.empty() \
		&& (sessionid[sessionid.size() - 1] == '\r' \
		|| sessionid[sessionid.size() - 1] == '\n')) {
			sessionid.erase(sessionid.size() - 1);
		}
	}
	return (sessionid);
}

std::string	SessionManager::setSessionId(std::string cookie) {
	std::string		sessionid;
	t_session_data	data;

	if (!cookie.empty()) {
		sessionid = extractSessionId(cookie);
		if (!sessionid.empty()) {
			// std::cout << "[setSessionId] Cookie header found. Session ID extracted: " << sessionid << std::endl;
			return (sessionid);
		} else {
			// std::cout << "[setSessionId] Cookie header found, but session ID extraction failed." << std::endl;
		}
	} else {
		// std::cout << "[setSessionId] No cookie header found in request." << std::endl;
	}
	// std::cout << "[setSessionId] Session ID not found or invalid. Set to empty." << std::endl;
	return ("");
}

std::string	SessionManager::getSetCookieHeaderValue(std::string sessionid) {
	return (SESSIONID_KEY + sessionid + "; Max-Age=3600; Path=/; HttpOnly");
	// m_accept.response.addHeader(HttpConst::HTTP_SET_COOKIE, "", "Secure");
	// m_accept.response.addHeader(HttpConst::HTTP_SET_COOKIE, "SameSite", "None;");
}
