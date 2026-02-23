#include "LocationConfig.hpp"
// LocationConfig::LocationConfig(): AConfig(e_location), m_api_point(""), m_priority(e_prefix_match) {}
AConfig	*LocationConfig::mp_default = new LocationConfig(location_conf);
// MainConfig::MainConfig(std::string file): AConfig(e_main), m_server()
LocationConfig::LocationConfig(): AConfig(), m_api_point(""), m_priority(e_prefix_match) { }

LocationConfig::LocationConfig(std::string file): AConfig(), m_api_point(""), m_priority(e_prefix_match) {
	FileParser								parser;

	parser.getFile(file);
	parser.setDefault(*this);
}

// LocationConfig::LocationConfig(std::string& api_point, std::string priority): AConfig(e_location), m_api_point(api_point), m_priority() {
LocationConfig::LocationConfig(std::string& api_point, std::string priority): AConfig(), m_api_point(api_point), m_priority() {
	setPriority(priority);
}
AConfig	*LocationConfig::getInstance()
{
	if (!mp_default)
		mp_default = new LocationConfig(location_conf);
	return mp_default;
}
LocationConfig::~LocationConfig() {}

void	LocationConfig::deleteConfig() {
	if (mp_default)
		delete mp_default;
}

LocationConfig	&LocationConfig::operator=(const LocationConfig &obj)
{
	if (this == &obj)
		return *this;
	m_api_point = obj.m_api_point;
	m_priority = obj.m_priority;
	replaceConfig(obj);
	return *this;
}

bool	LocationConfig::operator==(const LocationConfig& obj) {
	return (m_api_point == obj.m_api_point && m_priority == obj.m_priority && getConfigs() == obj.getConfigs());
}

t_priority	LocationConfig::getPriority() const {
	return m_priority;
}

void	LocationConfig::setPriority(std::string priority){
	if (priority == "=")
		m_priority = e_exactly;
	else if (priority == "^~")
		m_priority = e_preferential_prefix_match;
	// else if (priority == "~")
		// m_priority = e_regex_match_with_sensitive_case;
	// else if (priority == "~*")
		// m_priority = e_regex_match_with_insensitive_case;
	else if (priority == "location")
		m_priority = e_prefix_match;
	else
	{
		std::cerr << "DEBUG: location priority is wrong." << std::endl;
		Exception::handleInvalidFile();
	}
}

void	LocationConfig::printConfigs() const {
	std::cout << "<----------------Location---------------->" << std::endl;
	this->AConfig::printConfigs();
}

// location 중첩일때
void	LocationConfig::setConfig(const AConfig &obj){
	LocationConfig	*is_location;

	is_location = dynamic_cast<LocationConfig*>(const_cast<AConfig*>(&obj));
	if (is_location)
		*this = *is_location;
	else
		Exception::handleInvalidFile();
}

void	LocationConfig::setApiPoint(const std::deque<std::string> &deq, size_t pos)
{
	if (pos == (size_t) -1 || pos == 1 || (deq[0] == "location" && pos > 3))
		Exception::handleInvalidFile();
	setPriority(deq[pos - 2]);
	m_api_point = deq[pos - 1];
}

// void	LocationConfig::updateDefaultSettings() {
// 	Parser								parser("../utils/conf.d/requirements/location.nginx_keys");
// 	std::map<std::string, std::string>	ret;

// 	ret = parser.getMap();
// 	this->AConfig::updateDefault(ret);
// 	std::cout << "update Location Default Setting complete" << std::endl;
// }
void	LocationConfig::inheritConfig(const AConfig& obj) {
	if (dynamic_cast<ServerConfig *>(&(const_cast<AConfig&>(obj))))
	{
		addConfigIfNotExist(obj.getConfigs());
		addTypesIfNotExist(obj.getTypes());
		addErrorsIfNotExist(obj.getErrors());
		addIndexIfNotExist(obj.getIndex());
		addRedirectsIfNotExist(obj.getRedirects());
	}
	// else if (dynamic_cast<LocationConfig *>(&(const_cast<AConfig&>(obj))))
	else
		Exception::handleInvalidFile();
}

std::string	LocationConfig::getEndPoint() const {
	return m_api_point;
}

void	LocationConfig::checkValidConfigs(){
	AConfig::checkValidConfigs();
	// root 경로 검사
}

bool	LocationConfig::match(const std::string &api_point) const {
	if (m_priority == e_exactly)
		return api_point == m_api_point;
	// if (api_point.find(m_api_point) == 0)
		// std::cout <<"api: " << m_api_point << " matched" << std::endl;
	return (api_point.find(m_api_point) == 0);
}

void	LocationConfig::updateDefaultConfig() {
	AConfig								*ptr;

	ptr = getInstance();
	addConfigIfNotExist(ptr->getConfigs());
	addErrorsIfNotExist(ptr->getErrors());
	addTypesIfNotExist(ptr->getTypes());
	addIndexIfNotExist(ptr->getIndex());
	addMethodsIfNotExist(ptr->getMethods());
	addRedirectsIfNotExist(ptr->getRedirects());
}

std::string	LocationConfig::getContentType(const std::string &target) const {
	std::string	ret;

	try	{
		ret = findType(target);
	}
	catch(const std::exception& e) {
		return find("default_type");
	}
	return ret;
}

std::string	LocationConfig::getErrorPage(const std::string &target) const {
	return findErrorPage(target);
}

bool	LocationConfig::isValidMethod(const std::string &method) const {
	return (findMethodTarget(method) != (size_t)-1);
}

bool	LocationConfig::hasRedirect(std::string &redirect, std::string &status) const {
	std::vector<std::string>	v;

	v = getRedirects();
	if (v.size() < 2)
		return false;
	redirect = v[1];
	status = v[0];
	return true;
}

bool	LocationConfig::hasProxy(std::string &pass) const {
	pass = find("proxy_pass");
	// return true;
	return (pass != "-");
}

bool	LocationConfig::hasCgi(const std::string &api, std::string &script_name, std::string &path_info) const {
	path_info = api.substr(m_api_point.size());
	script_name = find("cgi_param_script_filename");
	return (script_name != "-");
}

bool	LocationConfig::hasAutoIndex(std::string api_point, std::string &path, std::string &status) const {
	std::string	root;

	root = find("alias");
	path = api_point.substr(m_api_point.size());
	if (root[root.size() - 1] != '/' && path[0] != '/')
		root += '/';
	else if (root[root.size() - 1] == '/' && path[0] == '/')
		path = path.substr(1);
	if (find("autoindex") == "off" || find("autoindex") == "-")
		return false;
	status = "200";
	if (path.find("/../") != (size_t(-1)))
	{
		status = "403";
	}
	else if (path.find("/..") != (size_t)-1 && path.find("/..") == path.size() - 3)
		status = "403";
	else if (Checker::isExecutableDirectory(root + path))
	{
		path = root + path;
		status = "200";
	}
	else
		status = "404";
	return true;
}

bool	LocationConfig::hasIndex(std::string api_point, std::string &path, std::string &status) const {
	std::string	root;

	root = find("alias");
	if (root[root.size() - 1] != '/')
		root += '/';
	path = api_point.substr(m_api_point.size());
	if (root[root.size() - 1] == '/' && path[0] == '/')
		path = path.substr(1);
	if (path.find("/../") != (size_t(-1)))
		status = "403";
	else if (path.find("/..") != (size_t)-1 && path.find("/..") == path.size() - 3)
		status = "403";
	else if (!Checker::isFileExist(root + path))
		status = "404";
	else if (Checker::isOneofFilesExist(root, path, getIndex()))
	{
		status = "200";
		return true;
	}
	else
		status = "404";
	if (status != "200" && (path = find("try_files")) != "-" && Checker::isFileExist(path))
	{
		status = "200";
		return true;
	}
	return false;
}
