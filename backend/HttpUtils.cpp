#include "HttpUtils.hpp"

/* ************************************************************************** */
/* ---------------------------------   OCCF  -------------------------------- */
/* ************************************************************************** */

/* public */
HttpUtils::HttpUtils() {}

HttpUtils::~HttpUtils() {}

/* ************************************************************************** */
/* ------------------------------    METHOD   ------------------------------- */
/* ************************************************************************** */

/* public */
const std::string	HttpUtils::getMethod(t_http_method method) {
	switch (method) {
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN_METHOD";
	}
}

const std::string	HttpUtils::getStatus(t_http_status status) {
	switch (status) {
		case OK_200:
			return "200";
		case CREATED_201:
			return "201";
		case MOVED_PERMANENTLY_301:
			return "301";
		case FOUND_302:
			return "302";
		case SEE_OTHER_303:
			return "303";
		case BAD_REQUEST_400:
			return "400";
		case UNAUTHORIZED_401:
			return "401";
		case NOT_FOUND_404:
			return "404";
		case METHOD_NOT_ALLOWED_405:
			return "405";
		case REQUEST_TIMEOUT_408:
			return "408";
		case LENGTH_REQUIRED_411:
			return "411";
		case REQUEST_ENTITY_TOO_LARGE_413:
			return "413";
		case REQUEST_URI_TOO_LONG_414:
			return "414";
		case INTERNAL_SERVER_ERROR_500:
			return "500";
		case BAD_GATEWAY_502:
			return "502";
		case SERVICE_UNAVAILABLE_503:
			return "503";
		case VERSION_NOT_SUPPORTED_505:
			return "505";
		default:
			return "UNKNOWN_STATUS";
	}
}

t_http_status	HttpUtils::getEnumFromStatus(const std::string& status_str) {
	if (status_str == "200")
		return OK_200;
	if (status_str == "201")
		return CREATED_201;
	if (status_str == "301")
		return MOVED_PERMANENTLY_301;
	if (status_str == "302")
		return FOUND_302;
	if (status_str == "303")
		return SEE_OTHER_303;
	if (status_str == "400")
		return BAD_REQUEST_400;
	if (status_str == "401")
		return UNAUTHORIZED_401;
	if (status_str == "404")
		return NOT_FOUND_404;
	if (status_str == "405")
		return METHOD_NOT_ALLOWED_405;
	if (status_str == "408")
		return REQUEST_TIMEOUT_408;
	if (status_str == "411")
		return LENGTH_REQUIRED_411;
	if (status_str == "413")
		return REQUEST_ENTITY_TOO_LARGE_413;
	if (status_str == "414")
		return REQUEST_URI_TOO_LONG_414;
	if (status_str == "500")
		return INTERNAL_SERVER_ERROR_500;
	if (status_str == "502")
		return BAD_GATEWAY_502;
	if (status_str == "503")
		return SERVICE_UNAVAILABLE_503;
	if (status_str == "505")
		return VERSION_NOT_SUPPORTED_505;
	return UNKNOWN_STATUS;
}

const std::string	HttpUtils::getStatusMessage(t_http_status status) {
	switch (status) {
		case OK_200:
			return "OK";
		case CREATED_201:
			return "Created";
		case MOVED_PERMANENTLY_301:
			return "Moved Permanently";
		case FOUND_302:
			return "Found";
		case SEE_OTHER_303:
			return "See Other";
		case BAD_REQUEST_400:
			return "Bad Request";
		case UNAUTHORIZED_401:
			return "Unauthorized";
		case NOT_FOUND_404:
			return "Not Found";
		case METHOD_NOT_ALLOWED_405:
			return "Method Not Allowed";
		case REQUEST_TIMEOUT_408:
			return "Request Timeout";
		case LENGTH_REQUIRED_411:
			return "Length Required";
		case REQUEST_ENTITY_TOO_LARGE_413:
			return "Request Entity Too Large";
		case REQUEST_URI_TOO_LONG_414:
			return "Request-URI Too Long";
		case INTERNAL_SERVER_ERROR_500:
			return "Internal Server Error";
		case BAD_GATEWAY_502:
			return "Bad Gateway";
		case SERVICE_UNAVAILABLE_503:
			return "Service Unavailable";
		case VERSION_NOT_SUPPORTED_505:
			return "HTTP Version not Supported";
		default:
			return "UNKNOWN_STATUS_MESSAGE";
	}
}

const std::string	HttpUtils::getStartline(t_http_startline startline) {
	switch (startline) {
		case METHOD:
			return "Method";
		case URI:
			return "URI";
		case VERSION:
			return "HTTP-Version";
		case STATUS_CODE:
			return "Status-Code";
		case STATUS_MESSAGE:
			return "Status-Message";
		default:
			return "UNKNWON_STARTLINE";
	}
}

const std::string	HttpUtils::getHeader(t_http_header header) {
	switch (header) {
		case HOST:
			return "Host";
		case DATE:
			return "Date";
		case CONTENT_TYPE:
			return "Content-Type";
		case CONTENT_LENGTH:
			return "Content-Length";
		case CONNECTION:
			return "Connection";
		case SET_COOKIE:
			return "Set-Cookie";
		case COOKIE:
			return "Cookie";
		case LOCATION:
			return "Location";
		default:
			return "UNKNOWN_HEADER";
	}
}
