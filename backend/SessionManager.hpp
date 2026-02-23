#ifndef SESSIONMANAGER_HPP
# define SESSIONMANAGER_HPP

# include <iostream>
# include <ctime> // time
# include <cstdlib> // rand
# include <map>

typedef struct s_session_data {
	std::string		user_data;
}	t_session_data;

typedef struct s_session {
	t_session_data	data;
	time_t			creation_time;
	time_t			expiration_time;
}	t_session;

typedef std::map<std::string, t_session> t_session_map;

class SessionManager {
	private:
		t_session_map	sessions;
	public:
		/* OCCF */
						SessionManager();
						~SessionManager();
		/* METHOD */
						std::string		generateNewSessionID();
		t_session_data	getSessionData(const std::string &session_id);
		std::string		createSession(const t_session_data &data);
		void			setSessionData(const std::string &session_id, const std::string &data);
		std::string		parseSessionID(const std::string &cookie_header_value);
		bool			deleteExpiredSession(const std::string &session_id);
		std::string 	manageExpiredSession();
		void			updateSessionTimeout(const std::string &session_id);
		bool			isValidSessionID(const std::string &session_id);
		void			printSessionMap() ;
		std::string		extractSessionId(const std::string& cookieHeader);
		std::string		setSessionId(std::string cookie);
		std::string		getSetCookieHeaderValue(std::string sessionid);
};


#endif 