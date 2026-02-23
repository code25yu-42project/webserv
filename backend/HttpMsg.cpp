#include "HttpMsg.hpp"

/* ************************************************************************** */
/* ------------------------------	 OCCF	------------------------------- */
/* ************************************************************************** */

/* public */
HttpMsg::HttpMsg() {
	m_http_msg.clear();
	m_body.clear();
	m_startline.clear();
	m_header.clear();
	m_header_order.clear();
	m_recv_type = NONE;
	m_sent_size = 0;
	m_http_utils = HttpUtils();
}

HttpMsg::~HttpMsg() {}

/* ************************************************************************** */
/* ------------------------------	 RECV/SEND	------------------------------ */
/* ************************************************************************** */

/* public */
// int	HttpMsg::recvBuffer(const int& from_fd, std::string &string, const size_t &buffer_size) {
// 	char	buffer[buffer_size];
// 	int		bytes_read;

// 	bytes_read = recv(from_fd, buffer, buffer_size, 0);
// 	if (bytes_read == -1) {
// 		// perror("read failed");
// 		// std::cout << "Recv error: Could not read Data." << std::endl;
// 	} else if (bytes_read == 0) {
// 		// std::cout << "Connection closed by peer." << std::endl;
// 	} else {
// 		buffer[bytes_read] = '\0';
// 		string = buffer;
// 		// std::cout << "------------------------------------------------" << std::endl;
// 		// std::cout << "[recv request/response]" << string.size() << std::endl;
// 		// std::cout << "read from FD " << from_fd << std::endl;
// 		// ServerUtils utils;
// 		// utils.printSockInfo(from_fd);
// 		// std::cout << string.substr(0, 1000) << std::endl;
// 		// std::cout << "------------------------------------------------" << std::endl;
// 	}
// 	return (bytes_read);
// }

int HttpMsg::recvBuffer(const int& from_fd,
                        std::string &out,
                        const size_t &buffer_size)
{
	std::vector<char>	buffer(buffer_size);
	int					bytes_read;

	bytes_read = recv(from_fd, &buffer[0], buffer_size, 0);
	if (bytes_read <= 0)
		return bytes_read;

	out.append(&buffer[0], bytes_read);
	return bytes_read;
}

int	HttpMsg::sendBuffer(const int& to_fd, const std::string& string, int &total_sent) {
	size_t	string_size;
	int		bytes_sent;

	string_size = string.size();
	bytes_sent = send(to_fd, string.c_str() + total_sent, string_size - total_sent, 0);
	if (bytes_sent == -1) {
		// std::cout << "Send error: Could not write Data." << std::endl;
	} else if (bytes_sent == 0) {
		// std::cout << "Connection closed by peer." << std::endl;
	} else {
		total_sent += bytes_sent;
		// std::cout << "------------------------------------------------" << std::endl;
		// std::cout << "[send request/response]" << string.size() << std::endl;
		// std::cout << "write to FD " << to_fd  <<std::endl;
		ServerUtils utils;
		utils.printSockInfo(to_fd);
		// std::cout << string.substr(0, 2000) << std::endl;
		// std::cout << "------------------------------------------------" << std::endl;
	}
	return (bytes_sent);
}

/* ************************************************************************** */
/* ------------------------------	 VALID	------------------------------ */
/* ************************************************************************** */

/* public */
std::string	HttpMsg::isValidRequest(const std::string &request, const int &header_size, const int &body_size) {
	int			begin, end;
	std::string	status;
	std::string	startline, header, body;

	// if (request.empty()) {
	// 	return (m_http_utils.getStatus(BAD_GATEWAY_502));
	// }
	begin = 0;
	status = m_http_utils.getStatus(OK_200);
	end = splitByDelim(request, "\r\n", startline, begin);
	// std::cout << "#startline" << std::endl << startline << std::endl;
	if (end < 0)
		return (status);
	status = isValidRequestStartline(startline);
	// std::cout <<"[isValidRequest] valid startline:" <<  status << std::endl;
	if (status != m_http_utils.getStatus(OK_200))
		return (status);
	end = splitByDelim(request, "\r\n\r\n", header, begin);
	// std::cout << "#header" << std::endl << header << std::endl;
	if (end < 0)
		return (status);
	status = isValidRequestHeader(header, header_size);
	// std::cout << "[isValidRequest] valid header:" << status << std::endl;
	if (status != m_http_utils.getStatus(OK_200))
		return (status);
	body = request.substr(begin, request.size() - begin);
	// std::cout << "#body" << std::endl << body << std::endl;
	status = isValidBody(body, header, body_size);
	// std::cout << "[isValidRequest] valid request" << status << std::endl;
	return (status);
}

std::string	HttpMsg::isValidResponse(const std::string &response, const int &header_size, const int &body_size) {
	int			begin, end;
	std::string	startline, header, body, status;

	if (response.empty()) 
		return (m_http_utils.getStatus(BAD_GATEWAY_502));
	begin = 0;
	status = m_http_utils.getStatus(OK_200);
	end = splitByDelim(response, "\r\n", startline, begin);
	// std::cout << "#startline" << std::endl << startline << std::endl;
	if (end < 0)
		return (status);
	status = isValidResponseStartline(startline);
	if (status != m_http_utils.getStatus(OK_200))
		return (status);
	end = splitByDelim(response, "\r\n\r\n", header, begin);
	// std::cout << "#header" << std::endl << header << std::endl;
	if (end < 0)
		return (status);
	status = isValidResponseHeader(header, header_size);
	if (status != m_http_utils.getStatus(OK_200))
		return (status);
	body = response.substr(begin, response.size() - begin);
	// std::cout << "#body" << std::endl << body << std::endl;
	status = isValidBody(body, header, body_size);
	// std::cout << "[isValidResponse] valid response" << std::endl;
	return (status);
}

/* private */
std::string	HttpMsg::isValidRequestStartline(const std::string &startline) {
	int			begin, end;
	std::string	token, default_error_status;

	default_error_status = m_http_utils.getStatus(BAD_REQUEST_400);
	if (startline.empty())
		return (default_error_status);
	begin = 0;
	end = splitByDelim(startline, " ", token, begin);
	if (end < 0)
		return (default_error_status);
	end = splitByDelim(startline, " ", token, begin);
	if (end < 0 && isValidUri(token) == false)
		return (default_error_status);
	if (token.size() > 2000)
		return (m_http_utils.getStatus(REQUEST_URI_TOO_LONG_414));
	token = startline.substr(begin, startline.size() - begin);
	if (isValidHttpVersion(token) == false) {
		return (default_error_status);
	}
	// http version error - 504 HTTP Version Not Supported.
	return (m_http_utils.getStatus(OK_200));
}

bool	HttpMsg::isValidUri(const std::string& uri) {
	int	end;

	end = 0;
	if (isValidPath(uri, end) == false)
		return (false);
	return (true);
}

bool	HttpMsg::isValidPath(const std::string &uri, int &end) {
	char	c;

	if (end >= static_cast<int>(uri.size())) 
		return (false);
	for (; end < static_cast<int>(uri.size()); ++end) {
		c = uri[end];
		if (std::isalnum(c) || \
		c == '/' || c == '.' || c == '-' || c == '_' || c == '~')  // path에서 허용되는 문자
			continue;
		else
			return (false);
	}
	return (true);
}

bool	HttpMsg::isValidHttpVersion(const std::string& version) {
	std::string	minor;
	size_t		cnt_dot, idx, size;

	if (version.size() < 6 || version.substr(0, 5) != "HTTP/")
		return (false);	
	minor = version.substr(5);
	size = minor.size();
	cnt_dot = 0;
	for (idx = 0; idx < size; ++idx) {
		if (minor[idx] == '.') {
			if (idx == 0 || idx == size - 1)
				return (false);
			++cnt_dot;
			if (cnt_dot > 1)
				return (false);
		}
		else if (!isdigit(minor[idx]))
			return (false);
	}
	return (true);
}

std::string	HttpMsg::isValidResponseStartline(const std::string &startline) {
	int			begin, end;
	std::string	token, default_error_status;

	default_error_status = m_http_utils.getStatus(BAD_GATEWAY_502);
	if (startline.empty())
		return (default_error_status);
	begin = 0;
	end = splitByDelim(startline, " ", token, begin);
	if (end < 0)
		return (default_error_status);
	if (isValidHttpVersion(token) == false)
		return (default_error_status);
	if (token != "HTTP/1.1" && token != "HTTP/1.0")
		return (m_http_utils.getStatus(VERSION_NOT_SUPPORTED_505));
	end = splitByDelim(startline, " ", token, begin);
	if (end < 0)
		return (default_error_status);
	if (isValidStatusCode(token) == false)
		return (default_error_status);
	token = startline.substr(begin, startline.size() - begin);
	if (isValidStatusMsg(token) == false)
		return (default_error_status);
	return (m_http_utils.getStatus(OK_200));
}

bool	HttpMsg::isValidStatusCode (const std::string &status_code) {
	if (status_code.size() != 3 || \
		!isdigit(status_code[0]) || !isdigit(status_code[1]) || !isdigit(status_code[2]))
		return (false);
	return (true);
}

bool	HttpMsg::isValidStatusMsg (const std::string &status_msg) {
	if (status_msg.empty())
		return (false);
	return (true);
}

std::string	HttpMsg::isValidRequestHeader(const std::string &headers, const int &header_size) {
	int			begin, end, begin_without_space;
	std::string	key, value, default_error_status;

	begin = 0;
	default_error_status = m_http_utils.getStatus(BAD_REQUEST_400);
	if (headers.size() > static_cast<size_t>(header_size))
		return (m_http_utils.getStatus(REQUEST_ENTITY_TOO_LARGE_413));
	while (1) {
		end = splitByDelim(headers, " :", key, begin);
		if (end < 0)
			break ; 
		end = splitByDelim(headers, "\r\n", value, begin);
		if (end < 0)
			break; 
		if (isValidKey(key) == false)
			return (default_error_status);
		begin_without_space = value.find_first_not_of(" \t");
		if (begin_without_space == static_cast<int>(std::string::npos))
			return (default_error_status);
		// std::cout << "#" << key << "#:#" << value << "#" <<std::endl;
	}
	return (m_http_utils.getStatus(OK_200));
}

bool HttpMsg::isValidKey (const std::string &key) {
	size_t	idx;

	if (key.empty())
		return (false);
	for (idx = 0; idx < key.size(); ++idx) {
		if (!std::isalnum(key[idx]) && key[idx] != '-') 
			return (false);
	}
	return (true);
}

std::string	HttpMsg::isValidResponseHeader(const std::string &headers, const int &header_size) {
	int			begin, end, begin_without_space;
	std::string	key, value, default_error_status;
	
	begin = 0;
	default_error_status = m_http_utils.getStatus(BAD_GATEWAY_502);
	if (headers.size() > static_cast<size_t>(header_size))
		return (m_http_utils.getStatus(REQUEST_ENTITY_TOO_LARGE_413));
	while (1) {
		end = splitByDelim(headers, " :", key, begin);
		if (end < 0)
			break ; 
		end = splitByDelim(headers, "\r\n", value, begin);
		if (end < 0)
			break; 
		if (isValidKey(key) == false)
			return (default_error_status);
		begin_without_space = value.find_first_not_of(" \t");
		if (begin_without_space == static_cast<int>(std::string::npos))
			return (default_error_status);
		// std::cout << "#" << key << "#:#" << value << "#" <<std::endl;
	}
	return (m_http_utils.getStatus(OK_200));
}

std::string HttpMsg::isValidBody(const std::string& body, const std::string &header, const int &body_size) {
	size_t		length;
	std::string	content_length;
	
	content_length = parseValue(header, m_http_utils.getHeader(CONTENT_LENGTH));
	if (body.size() > static_cast<size_t>(body_size))
		return (m_http_utils.getStatus(REQUEST_ENTITY_TOO_LARGE_413));
	length = std::atoi(content_length.c_str());
	if (length != 0 && body.size() != length)
		return (m_http_utils.getStatus(LENGTH_REQUIRED_411));
	return (m_http_utils.getStatus(OK_200));
}

/* ************************************************************************** */
/* ---------------------------------- PARSE --------------------------------- */
/* ************************************************************************** */

/* public */
void	HttpMsg::parseMsg(const std::string &http_msg, const t_msg_type type) {
	// clearMsg();
	parseStartline(http_msg, type);
	parseHeaders(http_msg);
	parseBody(http_msg);
	// ServerUtils util;
	// util.printMap(m_startline);
	// util.printVector(m_header_order);
	// util.printMap(m_header);
	// std::cout << m_body << std::endl;
}

void HttpMsg::parseStartline(const std::string &http_msg, const t_msg_type type) {
	std::istringstream	ss(http_msg);
	std::string			startline, method, uri, version, status, status_msg;

	std::getline(ss, startline, '\n'); // 정확한 구분자 사용
	startline.erase(startline.find_last_not_of("\r") + 1); // '\r' 제거
	std::istringstream startLineStream(startline);
	if (type == REQUEST) {
		std::getline(startLineStream, method, ' ');
		std::getline(startLineStream, uri, ' ');
		std::getline(startLineStream, version, ' ');
		m_startline[m_http_utils.getStartline(METHOD)] = method;
		m_startline[m_http_utils.getStartline(URI)] = uri;
		m_startline[m_http_utils.getStartline(VERSION)] = version;
	} else {
		std::getline(startLineStream, version, ' ');
		std::getline(startLineStream, status, ' ');
		std::getline(startLineStream, status_msg);
		m_startline[m_http_utils.getStartline(VERSION)] = version;
		m_startline[m_http_utils.getStartline(STATUS_CODE)] = status;
		m_startline[m_http_utils.getStartline(STATUS_MESSAGE)] = status_msg;
	}
}

/* private */
void HttpMsg::parseHeaders(const std::string &http_msg) {
	std::istringstream	ss(http_msg);
	std::string			line, key, value;

	if (!std::getline(ss, line) || line.empty())
		return;
	while (std::getline(ss, line)) {
		if (line.empty() || line == "\r")
			break;
		std::istringstream lineStream(line);
		if (std::getline(lineStream, key, ':')) {
			if (std::getline(lineStream, value)) {
				value.erase(0, value.find_first_not_of(" \t\n"));
				value.erase(value.find_last_not_of(" \t\n") + 1);
				m_header[key] = value;
				if (std::find(m_header_order.begin(), m_header_order.end(), key) == m_header_order.end())				
					m_header_order.push_back(key);
			}
		}
	}
}

void HttpMsg::parseBody(const std::string& request) {
	size_t	body_pos;

	body_pos = request.find("\r\n\r\n");
	if (body_pos != std::string::npos) {
		m_body = request.substr(body_pos + 4);  // "\r\n\r\n" 길이 = 4
	} else {
		m_body.clear();
	}
}

/* ************************************************************************** */
/* ------------------------------	GETSET	------------------------------- */
/* ************************************************************************** */

/* public */
std::string HttpMsg::getHttpMsg() {
	return (m_http_msg);
}

std::string HttpMsg::getBody() {
	return (m_body);
}

std::string HttpMsg::getStartlineValue(const std::string key) {
	if (m_startline.find(key) != m_startline.end())
		return (m_startline[key]);
	return ("");
}

std::string HttpMsg::getStatusCode() {
	if (m_startline.find(m_http_utils.getStartline(STATUS_CODE)) != m_startline.end())
		return (m_startline[m_http_utils.getStartline(STATUS_CODE)]);
	return ("");
}

std::string HttpMsg::getHeaderValue(const std::string key) {
	if (m_header.find(key) != m_header.end())
		return (m_header[key]);
	return ("");
}

std::vector<std::string> HttpMsg::getHeaderOrder() {
	return (m_header_order);
}

t_recv_type HttpMsg::getRecvType() {
	return (m_recv_type);
}

int HttpMsg::getSentSize() {
	return (m_sent_size);
}

std::string HttpMsg::getRedirect() {
	return (m_redirect);
}

void	HttpMsg::setHttpMsg(const std::string http_msg) {
	m_http_msg += http_msg;
}

void	HttpMsg::setBody(const std::string new_body) {
	m_body = new_body;
}

void	HttpMsg::setStartline(const std::string key, const std::string new_value) {
	m_startline[key] = new_value;
}

void	HttpMsg::setStatusCode(const std::string &status) {
	m_startline[m_http_utils.getStartline(STATUS_CODE)] = status;
}

void	HttpMsg::setHeader(const std::string key, const std::string new_value) {
	m_header[key] = new_value ;
	
}
void	HttpMsg::setRecvType(t_recv_type new_type) {
	m_recv_type = new_type;
}

void	HttpMsg::setSentSize(int new_size) {
	m_sent_size = new_size;
}

void HttpMsg::setRedirect(const std::string new_redirect) {
	m_redirect = new_redirect;
}

void HttpMsg::setCgi(const std::string new_cgi) {
	m_cgi = new_cgi;
}

void	HttpMsg::addSentSize(int new_size) {
	m_sent_size += new_size;
}

/* ************************************************************************** */
/* ------------------------------	MERGE	------------------------------- */
/* ************************************************************************** */

/* public */
std::string HttpMsg::mergeHttpMsg(t_msg_type type) {
	std::stringstream	http_msg, size;
	std::string			full_body;

	// ServerUtils util;
	// util.printVector(m_header_order);
	// util.printMap(m_header);
	if (!m_redirect.empty()) {
		// if (m_body.empty() && !getStartlineValue(m_http_utils.getStartline(URI)).find("/api") ){
		if (m_body.empty()){
			full_body =  "{\n\"redirect\":\n" + m_redirect + "\n}";
		} else  {
			full_body =  "{\n\"redirect\":\n" + m_redirect + ",\n\"body\":" + m_body + "}";
		}
		m_body = full_body;
	}
	size << m_body.size();
	setHeader(m_http_utils.getHeader(CONTENT_LENGTH), size.str());
	if (type == REQUEST)
		http_msg << m_startline[m_http_utils.getStartline(METHOD)] + " " + \
					m_startline[m_http_utils.getStartline(URI)] + " " + \
					m_startline[m_http_utils.getStartline(VERSION)] + "\r\n";
	else 
		http_msg << m_startline[m_http_utils.getStartline(VERSION)] + " " + \
					m_startline[m_http_utils.getStartline(STATUS_CODE)] + " " + \
					m_startline[m_http_utils.getStartline(STATUS_MESSAGE)] + "\r\n";
	
	for (std::vector<std::string>::iterator it = m_header_order.begin(); it != m_header_order.end(); ++it) {
		std::string key = *it;
		http_msg << key << ": " << m_header[key] << "\r\n";
	}
	http_msg << "\r\n";
		http_msg << m_body;
	// std::cout << "------------------------------------------------" << std::endl;
	// if (type == REQUEST)
	// std::cout << "		  [send request]" << m_body.size() << "		  " << std::endl;
	// else 
	// std::cout << "		  [send response]" << m_body.size() << "		  " << std::endl;
	// std::string printstr = http_msg.str().substr(0, 1000);
	// std::cout << printstr << std::endl;
	// std::cout << http_msg.str() << std::endl;
	// std::cout << "------------------------------------------------" << std::endl;
	return (http_msg.str());
}

/* ************************************************************************** */
/* ------------------------------	CREATE	------------------------------- */
/* ************************************************************************** */

/* public */
void	HttpMsg::createResponseStartline(HttpMsg &request, HttpMsg &response, std::string status){
	std::vector<std::string>	key;
	std::string					version,  msg;

	key.push_back(m_http_utils.getStartline(VERSION));
	key.push_back(m_http_utils.getStartline(STATUS_CODE));
	key.push_back(m_http_utils.getStartline(STATUS_MESSAGE));
	version = request.getStartlineValue(m_http_utils.getStartline(VERSION));
	if (status.empty())
		status = response.getStatusCode();
	msg = m_http_utils.getStatusMessage(m_http_utils.getEnumFromStatus(status));
	if (version.empty())
		version = "HTTP/1.1";
	response.setStartline(key[0], version);
	response.setStartline(key[1], status);
	response.setStartline(key[2], msg);
	// std::cout << response.getStartlineValue(m_http_utils.getStartline(STATUS_MESSAGE)) << std::endl;;
}

void	HttpMsg::createHeader(HttpMsg &request, HttpMsg &response, std::map<std::string, std::string> headers) {
	std::stringstream	ss_length;

	// std::cout << "THIS IS CREATE HEADER" << std::endl;
	// std::cout << "content type:" << headers[m_http_utils.getHeader(CONTENT_TYPE)] << std::endl;
	/* REQUIRED : Date, Content-Type, Content-Length, Connection */
	response.addHeader(m_http_utils.getHeader(DATE), getNowDate());
	if (!headers[m_http_utils.getHeader(CONTENT_TYPE)].empty())
		response.addHeader(m_http_utils.getHeader(CONTENT_TYPE), headers[m_http_utils.getHeader(CONTENT_TYPE)]);
	if (!response.getBody().empty())
		ss_length << response.getBody().size();
	if (!ss_length.str().empty())
		response.addHeader(m_http_utils.getHeader(CONTENT_LENGTH), ss_length.str());
	if (request.getHeaderValue(m_http_utils.getHeader(CONNECTION)) == "close")
		response.addHeader(m_http_utils.getHeader(CONNECTION), "close");
	else if (response.getHeaderValue(m_http_utils.getHeader(CONNECTION)).empty())
		response.addHeader(m_http_utils.getHeader(CONNECTION), "keep-alive");
	/* OPTIONAL : location, Set-Cookie (보류)*/
	if (!headers[m_http_utils.getHeader(LOCATION)].empty())
		response.addHeader(m_http_utils.getHeader(LOCATION), headers[m_http_utils.getHeader(LOCATION)]);
	if (!headers[m_http_utils.getHeader(SET_COOKIE)].empty())
		response.addHeader(m_http_utils.getHeader(SET_COOKIE), headers[m_http_utils.getHeader(SET_COOKIE)]);
	// if (!headers["Access-Control-Allow-Origin"].empty())
	// 	response.addHeader("Access-Control-Allow-Origin", headers["Access-Control-Allow-Origin"]);
	// if (!headers["Access-Control-Allow-Credentials"].empty())
		// response.addHeader("Access-Control-Allow-Credentials", headers["Access-Control-Allow-Credentials"]);

}

void HttpMsg::addHeader(const std::string &header_name, const std::string &header_value) {
	std::vector<std::string>::iterator it;

	for (it = m_header_order.begin(); it != m_header_order.end(); ++it) {
		if (*it == header_name) {
			m_header[header_name] = header_value;
			return ;
		}
	}
	m_header_order.push_back(header_name);
	m_header[header_name] = header_value;
}

/* ************************************************************************** */
/* ------------------------------	UTILS	------------------------------- */
/* ************************************************************************** */

/* public */
void HttpMsg::clearMsg() {
	m_http_msg.clear();
	m_startline.clear();
	m_header_order.clear();
	m_header.clear();
	m_body.clear();
	m_recv_type = NONE;
	m_sent_size = 0;
}

t_recv_type	HttpMsg::checkHttpType(const std::string& input) {
	size_t		startline_end, header_end, content_length, body_start, body_end;
	std::string	headers;

	// 스타트라인 검사
	startline_end = input.find("\r\n");
	if (startline_end == std::string::npos) {
		return (NONE);
	}
	// 헤더 검사
	header_end = input.find("\r\n\r\n", startline_end);
	headers = input.substr(startline_end + 2, header_end - startline_end - 2);
	content_length = getContentLength(headers);
	if (header_end == std::string::npos && content_length != 0) {
		return (STARTLINE_COMPLETE);
	}
	// Content-Length가 있는 경우, 본문 검사
	if (content_length > 0) {
		body_start = header_end + 4;
		body_end = input.length();
		if (body_end - body_start < content_length) {
			return (HEADER_COMPLETE);
		} else {
			return (BODY_COMPLETE); 
		}
	}
	return (BODY_COMPLETE);
}

/* private */
int	HttpMsg::splitByDelim(const std::string &input, std::string delim, std::string &token, int &begin) {
	int	end;

	end = input.find(delim, begin);
	if (end != static_cast<int>(std::string::npos)) { // delim을 찾은 경우
		token = input.substr(begin, end - begin);
		begin = end + delim.size();
	}
	else { // delim을 찾지 못한 경우
		if (begin < static_cast<int>(input.size())) { // 남은 문자열이 있는 경우
			token = input.substr(begin);
			begin = input.size();
			end = input.size();
		}
		else { // 남은 문자열이 없는 경우 
			token.clear();
			end = -1;
		}
	}
	return (end);
}

std::string	HttpMsg::parseValue(const std::string& headers, const std::string key) {
	int		begin, end;
	
	begin = headers.find(key);
	if (begin != static_cast<int>(std::string::npos)) {
		end = headers.find("\r\n", begin);
		return (headers.substr(begin + key.size(), end - begin + key.size()));
	}
	return ("");
}

std::string	HttpMsg::getNowDate(){
	char		buffer[100];
	std::time_t	now;
	std::tm		*gmt;
	
	now = std::time(NULL);
	gmt = std::gmtime(&now);
	if (gmt) {
		std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
		return (std::string(buffer));
	}
	return ("");
}

size_t HttpMsg::getContentLength(const std::string& headers) {
	size_t		content_length_pos, value_pos, value_end_pos, content_length;
	std::string	content_length_str; 

	content_length_pos= headers.find("Content-Length: ");
	if (content_length_pos == std::string::npos) {
		return (0); // Content-Length가 없으면 0 반환
	}
	value_pos = content_length_pos + 15; // "Content-Length: "의 길이만큼 이동
	value_end_pos = headers.find("\r\n", value_pos);
	if (value_end_pos == std::string::npos) {
		return (0); // 헤더가 끝나지 않음
	}
	content_length_str = headers.substr(value_pos, value_end_pos - value_pos);
	std::stringstream ss(content_length_str);
	content_length = 0;
	ss >> content_length;
	return (content_length);
}
