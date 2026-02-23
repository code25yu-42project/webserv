#include "Controller.hpp"

/* ************************************************************************** */
/* ---------------------------------  OCCF  --------------------------------- */
/* ************************************************************************** */

/* public */
Controller::Controller(t_accept &accept, ServerConfig &serv_config, SessionManager &session) \
: m_serv_config(serv_config), m_session(session), m_accept(accept) {
	m_model = Model();
	m_view = View();
	m_cgi = Cgi();
	m_util = ServerUtils();
	m_http_utils = HttpUtils();
	m_uri = m_accept.request.getStartlineValue(m_http_utils.getStartline(URI));
	m_sessionid = session.setSessionId(m_accept.request.getHeaderValue(m_http_utils.getHeader(COOKIE)));
}

Controller::~Controller() {}

/* ************************************************************************** */
/* ---------------------------------  Create  ------------------------------- */
/* ************************************************************************** */

/* public */
void	Controller::createResponse() {
	// m_accept.response.setStatusCode(m_http_utils.getStatus(NOT_FOUND_404)); // IsValidRequest 테스트용
	// m_accept.request.setStartline(m_http_utils.getStartline(METHOD), HttpConst::HTTP_POST); // isValidMethod 테스트용
	// std::cout << "[CREATE RESPONSE].." << m_accept.response.getStatusCode() << std::endl;
	if (m_accept.response.getStatusCode() != m_http_utils.getStatus(OK_200)) {
		// std::cout << "> INVALID REQUEST" << std::endl;
		createErrPageResponse("");
		return ; 
	}
	m_method = m_accept.request.getStartlineValue(m_http_utils.getStartline(METHOD));
	// std::cout << "URI:" << m_uri << std::endl << "METHOD:" << m_method << std::endl;
	if (m_serv_config.getUriSetting(m_uri, m_loc_config)== false) { // 2. uri not exist 응답처리
		// std::cout << "> INVALID URI" << std::endl;
		createErrPageResponse(m_http_utils.getStatus(NOT_FOUND_404));
	} else if (m_loc_config.isValidMethod(m_method) == false){ // 3. method not allowed 응답처리
		// std::cout << "> INVALID METHOD" << std::endl;
		createErrPageResponse(m_http_utils.getStatus(METHOD_NOT_ALLOWED_405));
	} else { // 4. valid uri, method에 대한 응답처리 
		if (createSessionResponse() == true && m_accept.response.getStatusCode() != m_http_utils.getStatus(OK_200)) {
			createErrPageResponse(m_accept.response.getStatusCode());
			return ;
		}
		m_accept.sessionid = m_sessionid;
		// m_uri = m_loc_config.getEndPoint(); 
		try {
			createUriResponse();
		} catch (std::exception & e) {
			// std::cout << "HERE" << std::endl;
		}
	}
	// std::cout << "create m_accept.Response Complete!" << std::endl;
	// session.printSessionMap();
	// m_accept.response.printHeaders();
} 

/* private */
void	Controller::createUriResponse() {
	bool	has_response;

	// std::cout << "CREATE URI RESPONSE.." << m_accept.response.getStatusCode() << std::endl;
	// std::cout << "end_point: " << m_loc_config.getEndPoint() << std::endl;
	if (createRedirectResponse() == true) {
		has_response = true;
	} else if (createProxyResponse() == true) {
		return ;
	} else if (createAutoIndexResponse() == true) {
		has_response = true;
	} else if (createIndexResponse() == true) {
		has_response = true;
	} 
	if (createCgiResponse() == true)
		has_response =  true;
	else if (has_response == false)
		createErrPageResponse(m_accept.response.getStatusCode());
	// if (has_response == false) {
	// 	if (m_accept.response.getStatusCode() == m_http_utils.getStatus(OK_200))
	// 		createErrPageResponse(m_http_utils.getStatus(NOT_FOUND_404));
	// 	else 
	// 		createErrPageResponse(m_accept.response.getStatusCode());
	// }
	// std::cout << "createUriResponse Complete!" << std::endl;
	return ;
}

bool	Controller::createRedirectResponse() {
	std::string							redirect, status;
	std::map<std::string, std::string>	headers;

	if (m_loc_config.hasRedirect(redirect, status) == true) {
		// std::cout << ">> REDIRECT:" << m_accept.response.getStatusCode() <<  std::endl;
		// std::cout << "redirect: " << redirect << std::endl;
		headers[m_http_utils.getHeader(HOST)] = m_accept.request.getHeaderValue(m_http_utils.getHeader(HOST));
		m_accept.response.setRedirect(m_view.redirectToJson(redirect));
		m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, status);
		return (true);
	}
	m_accept.response.setStatusCode(status);
	return (false);
}

bool	Controller::createProxyResponse() {
	std::string	proxy_pass; 

	if (m_loc_config.hasProxy(proxy_pass) == true) {
		m_accept.proxy_pass = proxy_pass;
		// std::cout << ">> PROXY PASS:" << m_accept.response.getStatusCode() <<  std::endl;
		// std::cout << "proxy_pass:" << m_accept.proxy_pass << std::endl;
		return (true);
	}
	// std::cout << "proxy_pass:" << m_accept.proxy_pass << std::endl;
	return (false);
}

bool	Controller::createAutoIndexResponse() {
	std::string							path, status;
	std::map<std::string, std::string>	headers;
	t_vec_vec							dir_vec;

	if (m_loc_config.hasAutoIndex(m_uri, path, status) == true && status == m_http_utils.getStatus(OK_200)) { // path가 안 나옴 
		m_accept.response.setStatusCode(status);
		// std::cout << ">> AUTOINDEX:" << m_accept.response.getStatusCode() <<  std::endl;
		// std::cout << "path: " << path << std::endl;
		// path = "42_frontend/src/assets"; // 테스트용
		// status = m_http_utils.getStatus(OK_200); // 테스트용
		if (path.empty()) {
			m_accept.response.setBody("{\"Error\": \"Please provide the directory path for autoindex browsing.\"}");
		}
		// std::cout << 1 << std::endl;
		dir_vec = m_model.getDirVec(path);
		if (dir_vec.empty()) {
		// std::cout << 2 << std::endl;
			m_accept.response.setBody("{\"Error\": \"Failed to open the directory\"}");
		} else {
		// std::cout << 3 << std::endl;
			m_accept.response.setBody(m_view.dirToJson(dir_vec));
		}
		// std::cout << 4 << std::endl;
		headers[m_http_utils.getHeader(CONTENT_TYPE)] = m_loc_config.getContentType("json");
		// std::cout << 5 << std::endl;
		m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, "");
		return (true);
	}
	m_accept.response.setStatusCode(status);
	return (false);
}

bool	Controller::createIndexResponse() {
	std::string							path, status, extension;
	std::map<std::string, std::string>	headers;

	// std::cout << "M_URI:" << m_uri << std::endl;
	if (m_loc_config.hasIndex(m_uri, path, status) == true) {
		// if (m_accept.response.getStatusCode() == m_http_utils.getStatus(OK_200))
			m_accept.response.setStatusCode(status);
		// std::cout << ">> INDEX: " << m_accept.response.getStatusCode() <<  std::endl;
		// std::cout << "path: " << path << std::endl;
		// if (m_uri == "/40x.html")
			// path = "www/html/404.html";
		m_accept.response.setBody(m_util.getFileToStr(path));
		extension = m_util.getFileExtension(path);
		headers[m_http_utils.getHeader(CONTENT_TYPE)] = m_loc_config.getContentType(extension);
		m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, "");
		return (true);
	}
	// std::cout << "Index false path: " << path << std::endl;
	return (false);
}

bool	Controller::createCgiResponse() {
	std::string							script_name, path_info, user_id;
	std::map<std::string, std::string>	headers;

	if (m_loc_config.hasCgi(m_uri, script_name, path_info) == true) {
		// std::cout << ">> CGI: " << m_uri << std::endl;
		if (m_method == m_http_utils.getMethod(POST))
			m_accept.response.setStatusCode(m_http_utils.getStatus(CREATED_201));
		try {
			user_id = m_view.extractKey(m_session.getSessionData(m_sessionid).user_data, "\"user_id\"");
			// std::cout << "user_id: " << user_id << std::endl;
			m_accept.cgi_fd = executeCgi(m_accept.request, script_name, path_info, user_id);
		} catch (std::exception &e) {
			// std::cout << "cgi error " << std::endl;
			createErrPageResponse(m_http_utils.getStatus(INTERNAL_SERVER_ERROR_500));
		}
		if (m_accept.cgi_fd.first == -1 || m_accept.cgi_fd.second == -1) {
		} else {
			headers[m_http_utils.getHeader(CONTENT_TYPE)] = m_loc_config.getContentType("json");
			m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, "");
		}
		return (true);
	} 
	return (false);
}

std::pair<int, int> 	Controller::executeCgi(HttpMsg request, std::string script_name, std::string path_info, std::string user_id) {
	std::pair<int, int> 		cgi_fd;
	std::string					uri, method, extension;
	std::vector<std::string>	env;
	HttpUtils					http_utils;

	cgi_fd.first = -1;
	cgi_fd.second = -1;
	uri = request.getStartlineValue(http_utils.getStartline(URI));
	method = request.getStartlineValue(http_utils.getStartline(METHOD));
	// std::cout << "uri:" << uri << std::endl << "method:" << method << std::endl;
	// request.setBody("{\"user_id\": \"1\",\n\"content\": \"thisistest\"\n}");
	// request.setBody("{\"user_id\": \"1\", \"file\": \"this is file content\"}");
	env = m_cgi.createCgiEnv(request, script_name, path_info, request.getBody(), user_id);
	ServerUtils utils;
	// std::cout << "CGI_ENV" << std::endl;
	// utils.printVector(env);
	// std::cout << uri << " CGI Input:" << std::endl << request.getBody() << std::endl;
	ServerUtils util;
	extension = util.getFileExtension(script_name);
	if (extension == "py")
		return(m_cgi.executeCgi(PYTHON_PATH, script_name, env));
	else if (extension == "php")
		return(m_cgi.executeCgi(PHP_PATH, script_name, env));
	return (cgi_fd);
}

void	Controller::createErrPageResponse(std::string status) {
	std::string							err_page_uri, proxy_pass, location_header;
	std::map<std::string, std::string>	headers;
	LocationConfig						err_config;

	if (!status.empty())
		m_accept.response.setStatusCode(status);
	m_method = m_http_utils.getMethod(GET);
	std::cout << "> CREATE ERROR PAGE:" << m_accept.response.getStatusCode() << std::endl;
	try {
		err_page_uri = m_serv_config.getErrorPage(m_accept.response.getStatusCode());
		// std::cout << "err_page_uri:" << err_page_uri << std::endl;
		if (m_serv_config.getUriSetting(err_page_uri, err_config) == false) { // try 처리 물어보기 
			// std::cout << "uri setting doesn't exist." << std::endl;
			m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, m_accept.response.getStatusCode());
			// headers[location_header] = ERR_PAGE_URI;
		} else if (err_config.isValidMethod(m_method) == false){ // 3. method not allowed 응답처리
			// std::cout << "> INVALID METHOD" << std::endl;
			m_accept.response.setStatusCode(m_http_utils.getStatus(METHOD_NOT_ALLOWED_405));
		} else {
			if (!m_uri.find("/api")) {
				location_header = m_http_utils.getHeader(LOCATION);
				headers[location_header] = err_page_uri;
				m_accept.response.setRedirect(m_view.redirectToJson(err_page_uri));	
				m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, m_accept.response.getStatusCode());
			} else {
				location_header = m_http_utils.getHeader(LOCATION);
				headers[location_header] = err_page_uri;
				m_uri = err_page_uri;
				m_loc_config = err_config;
				// std::cout << "m_uri:" << m_uri << std::endl;
				// m_accept.response.setStatusCode(m_http_utils.getStatus(OK_200));
				m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, m_accept.response.getStatusCode());
				try {
					createUriResponse();
				} catch(std::exception &e) {
					// std::cout << "HERE" << std::endl;
				}
			}	
		}

		
	} catch (std::exception &e) {
		// std::cout << "err_page_uri doesn't exist." << std::endl;
		// headers[location_header] = ERR_PAGE_URI;
	}

	// 에러페이지의 uri에 대한 추가검사

	// location으로 처리 
	// return (proxy_pass);
}

bool Controller::createSessionResponse() {
	bool		is_valid_session;
	std::string	user_data, sessionid;

	// 만료된 세션 관리 시작
	if (!m_uri.find("/api")) {
		// std::cout << "[createSessionResponse] Expired sessions management started..." << std::endl;
		// m_session.printSessionMap();
		// 만료된 세션 처리
		while (!(user_data = m_session.manageExpiredSession()).empty()) {
			// std::cout << "[createSessionResponse] Expired session found: " << user_data << std::endl;
			// std::cout << m_view.extractKey(user_data, "\"user_id\"") << std::endl;
			m_view.removeJsonObject(USER_PATH, user_data);
		}
		// 만료된 세션 관리 종료
		// std::cout << "[createSessionResponse] Expired sessions management completed." << std::endl;
		// m_session.printSessionMap();
	}
	sessionid = m_accept.request.getHeaderValue(m_http_utils.getHeader(COOKIE)); // 세션 아이디가 요청에 포함되어 있는지 확인
	if (sessionid.empty()) {  // 요청에 세션 아이디가 없음
		// std::cout << "[createSessionResponse] No session ID in request." << std::endl;
		return (handleInvalidSessionRequest());
	} else {  // 요청에 세션 아이디가 있음
		m_sessionid = m_session.extractSessionId(sessionid);	
		// std::cout << "[createSessionResponse] Session ID: " << m_sessionid << std::endl;
		is_valid_session = m_session.isValidSessionID(m_sessionid); // 세션이 유효한지 확인
		if (!is_valid_session) {  // 세션이 만료됨
			// std::cout << "[createSessionResponse] Invalid session ID (expired)." << std::endl;
			return (handleInvalidSessionRequest());
		} else {  // 세션이 유효함
			// std::cout << "[createSessionResponse] Valid session ID." << std::endl;
			return (handleValidSessionRequest());
		}
	}
	// 기본적으로 반환하지 않음
	return (false);
}

bool	Controller::handleValidSessionRequest() {
	std::string	user_data, redirect, status, set_cookie_value;

	// 세션 타임아웃 갱신
	if (!m_uri.find("/api"))
		m_session.updateSessionTimeout(m_sessionid);
	set_cookie_value = m_session.getSetCookieHeaderValue(m_sessionid);
	m_accept.response.addHeader(m_http_utils.getHeader(SET_COOKIE), set_cookie_value);
	// std::cout << "[ValidSession] Valid session found. URI: " << m_uri << std::endl;
	user_data = m_session.getSessionData(m_sessionid).user_data;
	if (!user_data.empty()) { // user data 가 있는 유효한 세션 
		// std::cout << "[ValidSession] User data found: " << user_data << std::endl;
	} 
	return (false);
}

bool	Controller::handleInvalidSessionRequest() {
	std::map<std::string, std::string>	headers;

	// std::cout << "URI: " << m_uri << std::endl;
	if (!m_uri.find("/api/chatroom")) {
		// std::cout << "[InvalidSession] Session expired. URI: /chatroom. Redirecting to /main." << std::endl;
		m_accept.response.setStatusCode(m_http_utils.getStatus(UNAUTHORIZED_401));
		m_model.createResponseHeaders(m_accept.request, m_accept.response, headers, "");
		return (true);
	}
	return (false);
}
