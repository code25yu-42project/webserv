#ifndef HTTPUTILS_HPP
# define HTTPUTILS_HPP

#include <iostream>

typedef enum e_http_method {
	GET,
	POST,
	DELETE,
	UNKNOWN_METHOD
} t_http_method;

// HTTP Status Code
typedef enum e_http_status {
	OK_200,
	CREATED_201,
	MOVED_PERMANENTLY_301,
	FOUND_302,
	SEE_OTHER_303,
	BAD_REQUEST_400,
	UNAUTHORIZED_401,
	NOT_FOUND_404,
	METHOD_NOT_ALLOWED_405,
	REQUEST_TIMEOUT_408,
	LENGTH_REQUIRED_411,
	REQUEST_ENTITY_TOO_LARGE_413,
	REQUEST_URI_TOO_LONG_414,
	INTERNAL_SERVER_ERROR_500,
	BAD_GATEWAY_502,
	SERVICE_UNAVAILABLE_503,
	VERSION_NOT_SUPPORTED_505,
	UNKNOWN_STATUS
} t_http_status;

// HTTP Headers
typedef enum e_http_header {
	HOST,
	DATE,
	CONTENT_TYPE,
	CONTENT_LENGTH,
	CONNECTION,
	SET_COOKIE,
	COOKIE,
	LOCATION,
	UNKNOWN_HEADER
} t_http_header;

// HTTP startline
typedef enum e_http_startline {
	METHOD,
	URI,
	VERSION,
	STATUS_CODE,
	STATUS_MESSAGE
}	t_http_startline;

class HttpUtils {
	public:
		/* OCCF */
							HttpUtils();
							~HttpUtils();
		/* METHOD */
		const std::string	getMethod(t_http_method method);
		const std::string	getStatus(t_http_status status);
		t_http_status		getEnumFromStatus(const std::string& status_str);
		const std::string	getStatusMessage(t_http_status status);
		const std::string	getStartline(t_http_startline startline);
		const std::string	getHeader(t_http_header header);
};

#endif
