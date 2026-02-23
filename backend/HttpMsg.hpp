#ifndef HTTPMSG_HPP
# define HTTPMSG_HPP

# include <cstdlib>
# include <ctime>  

# include "HttpUtils.hpp"
# include "ServerUtils.hpp"

typedef enum e_bool {
	FALSE,
	TRUE
}	t_bool;

typedef enum e_msg_type {
	REQUEST,
	RESPONSE
}	t_msg_type;

typedef enum e_recv_type {
	NONE,
	STARTLINE_COMPLETE,
	HEADER_COMPLETE,
	BODY_COMPLETE
}	t_recv_type;

class HttpMsg {
	private:
		std::string							m_http_msg, m_body, m_redirect, m_cgi;
		std::map<std::string, std::string>	m_startline, m_header;
		std::vector<std::string>			m_header_order; // 헤더명
		t_recv_type							m_recv_type;
		int									m_sent_size;
		HttpUtils							m_http_utils;
		/* VALID */
		std::string			isValidRequestStartline(const std::string &startline);
		bool				isValidUri(const std::string& uri);
		bool				isValidPath(const std::string &uri, int &next_pos);
		bool				isValidHttpVersion(const std::string &version);
		
		std::string			isValidResponseStartline(const std::string &startline);
		bool				isValidStatusCode (const std::string &status_code);
		bool				isValidStatusMsg (const std::string &status_msg);
		
		std::string			isValidRequestHeader(const std::string &headers, const int &header_size);
		bool				isValidKey (const std::string &key);
		std::string			isValidResponseHeader(const std::string &headers, const int &header_size);
		std::string			isValidBody(const std::string& body, const std::string &header, const int &body_size);
		/* PARSE*/
		void				parseHeaders(const std::string& http_msg);
		void				parseBody(const std::string& http_ms);
		/* UTILS */
		int					splitByDelim(const std::string &input, std::string delim, std::string &token, int &begin);
		std::string			parseValue(const std::string& headers, const std::string key);
		std::string			getNowDate();
		size_t				getContentLength(const std::string& headers);
	public:
		/* OCCF */
							HttpMsg();
		virtual				~HttpMsg();
		/* READ */
		int					recvBuffer(const int& from_fd, std::string &string, const size_t &buffer_size);
		int					sendBuffer(const int& to_fd, const std::string& string, int &total_sent);
		/* VALID */
		std::string			isValidRequest(const std::string &request, const int &header_size, const int &body_size);
		std::string			isValidResponse(const std::string &response, const int &header_size, const int &body_size);
		/* PARSE */
		void				parseMsg(const std::string &http_msg, const t_msg_type type);
		void				parseStartline(const std::string &http_msg, const t_msg_type type) ;
		/* GETSET */
		std::string			getHttpMsg();
		std::string			getBody();
		std::string			getStartlineValue(const std::string key);
		std::string			getStatusCode();
		std::string			getHeaderValue(const std::string key);
		std::vector<std::string> getHeaderOrder();
		t_recv_type			getRecvType();
		int					getSentSize();
		std::string			getRedirect();
		void		 		setHttpMsg(const std::string http_msg);
		void				setBody(const std::string new_body);
		void				setStartline(const std::string key, const std::string new_value);
		void				setStatusCode(const std::string &status);
		void				setHeader(const std::string key, const std::string new_value);
		void				setRecvType(t_recv_type new_type);
		void				setSentSize(int new_size);
		void				setRedirect(const std::string new_redirect);
		void				setCgi(const std::string new_cgi);
		void				addSentSize(int new_size);
		/* MERGE */
		std::string			mergeHttpMsg(t_msg_type type);
		/* CREATE */
		void				createResponseStartline(HttpMsg &request, HttpMsg &response, std::string status);
		void 				createHeader(HttpMsg &request, HttpMsg &response, std::map<std::string, std::string> headers);
		void				addHeader(const std::string &header_name, const std::string &header_value) ;
		/* UTILS */
		void				clearMsg();
		t_recv_type			checkHttpType(const std::string& input);
};

#endif