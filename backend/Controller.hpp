#ifndef CONTROLLER_HPP
# define CONTROLLER_HPP

# include <iostream>

# include "../config/LocationConfig.hpp"
# include "Model.hpp"
# include "View.hpp"
# include "Cgi.hpp"
# include "SessionManager.hpp"

class Controller {
	private:
		Model				m_model;
		View				m_view;
		Cgi					m_cgi;
		ServerUtils			m_util;
		HttpUtils			m_http_utils ;
		std::string			m_uri, m_method, m_sessionid;
		ServerConfig		&m_serv_config;
		SessionManager		&m_session;
		LocationConfig		m_loc_config;
		t_accept			&m_accept;
		void				createUriResponse();
		bool				createRedirectResponse();
		bool				createProxyResponse();
		bool				createAutoIndexResponse();
		bool				createIndexResponse();
		bool 				createCgiResponse();
		std::pair<int, int>	executeCgi(HttpMsg request, std::string script_name, std::string path_info, std::string user_id);
		void				createErrPageResponse(std::string status);
		bool				createSessionResponse();
		bool				handleValidSessionRequest();
		bool				handleInvalidSessionRequest();
	public:
							Controller(t_accept &accept, ServerConfig &serv_config, SessionManager &session);
							~Controller();
		void				createResponse();
};

#endif 