#include "ServerConfig.hpp"

AConfig	*ServerConfig::mp_default = new ServerConfig(server_conf);
ServerConfig::ServerConfig(): AConfig(), m_locations() { }

ServerConfig::ServerConfig(std::string file): AConfig(), m_locations() {
	FileParser								parser;

	parser.getFile(file);
	parser.setDefault(*this);
}
ServerConfig::~ServerConfig() { }

AConfig	*ServerConfig::getInstance()
{
	if (mp_default == NULL)
		mp_default = new ServerConfig(server_conf);
	return mp_default;
}

void	ServerConfig::deleteConfig() {
	if (mp_default)
		delete mp_default;
	mp_default = NULL;
}

void	ServerConfig::clear() {
	this->AConfig::clear();
	clearLocation();
}

ServerConfig&	ServerConfig::operator=(const ServerConfig &obj) {
	if (this == &obj)
		return *this;
	replaceConfig(obj);
	// setScope(obj.getScope());
	for (size_t priority = 0; priority < lnum; priority++)
	{
		m_locations[priority].clear();
		m_locations[priority].reserve(obj.m_locations[priority].size());
		m_locations[priority] = obj.m_locations[priority];
	}
		// setLocation(priority, obj.m_locations[priority]);
	return *this;
}

void	ServerConfig::clearLocation() {
	for (size_t i = 0; i < lnum; i++)
		m_locations[i].clear();
}
void	ServerConfig::updateDefaultConfig() {
	AConfig								*ptr;

	ptr = getInstance();
	addConfigIfNotExist(ptr->getConfigs());
	addErrorsIfNotExist(ptr->getErrors());
	addTypesIfNotExist(ptr->getTypes());
	addIndexIfNotExist(ptr->getIndex());
	addRedirectsIfNotExist(ptr->getRedirects());
	addAddressIfNotExist(ptr->getIPs(), ptr->getPort());
	for (size_t i = 0; i < lnum; i++)
	{
		for (size_t j = 0; j < m_locations[i].size(); j++)
			m_locations[i][j].updateDefaultConfig();
	}
}

void	ServerConfig::printConfigs() const {
	std::cout << "<----------------SERVER---------------->" << std::endl;
	this->AConfig::printConfigs();

	for (size_t i = 0; i < lnum; i++)
	{
		for (size_t j = 0; j < m_locations[i].size(); j++)
			m_locations[i][j].printConfigs();
	}
}

void	ServerConfig::setConfig(const AConfig &obj) {
	ServerConfig	*is_server;
	LocationConfig	*is_location;

	is_server = dynamic_cast<ServerConfig*>(const_cast<AConfig*>(&obj));
	is_location = dynamic_cast<LocationConfig*>(const_cast<AConfig*>(&obj));
	if (is_server)
		*this = *is_server;
	else if (is_location)
		addNewLocation(is_location->getPriority(), *is_location);
	else
		Exception::handleInvalidFile();
}

// size_t	ServerConfig::getSizeOfLocations() const {
// 	return m_locations.size();
// }

void	ServerConfig::setLocation(size_t priority, const std::vector<LocationConfig>& obj) {
	m_locations[priority].clear();
	m_locations[priority] = obj;
}

void	ServerConfig::inheritConfig(const AConfig& obj) {
	if (dynamic_cast<MainConfig*>(&(const_cast<AConfig&>(obj))))
	{
		addConfigIfNotExist(obj.getConfigs());
		addTypesIfNotExist(obj.getTypes());
		addErrorsIfNotExist(obj.getErrors());
		addIndexIfNotExist(obj.getIndex());
		addRedirectsIfNotExist(obj.getRedirects());
	}
	else
		Exception::handleInvalidFile();
	for (size_t i = 0; i < lnum; i++)
	{
		for (size_t j = 0; j < m_locations[i].size(); j++)
			m_locations[i][j].inheritConfig(*this);
	}
}

std::string	ServerConfig::getErrorPage(const std::string &target) const {
	return findErrorPage(target);
}
// bool	ServerConfig::isValidKeyword(std::string key) const {
// 	if (key == "aa")
// 		return true;
// 	return false;
// }

// bool	ServerConfig::isValidConfigs() const {
// 	std::map<std::string, std::string>::const_iterator	it, ite;

// 	if (getConfigSize() != )
// 		return false;
// 	ite = getEndIterator();
// 	for (it = getBeginIterator(); it != ite; it++)	{
// 		if (!isValidKeyword(it->first))
// 			return false;
// 	}
// 	return true;
// }

void	ServerConfig::addNewLocation(std::string& api_point, std::string priority) {
	LocationConfig	lconfig(api_point, priority);

	m_locations[lconfig.getPriority()].push_back(lconfig);
}

void	ServerConfig::addNewLocation(size_t priority, const LocationConfig& obj) {
	m_locations[priority].push_back(obj);
}

// void	ServerConfig::updateDefaultSettings() {
// 	Parser								parser("../utils/conf.d/requirements/server.nginx_keys");
// 	std::map<std::string, std::string>	ret;

// 	ret = parser.getMap();
// 	// std::cout << "update Server Default Setting complete" << std::endl;
// }

// void	ServerConfig::inheritConfig(const AConfig& obj) {
// 	addConfigIfNotExist(obj.getConfigs());
// }
// std::string	ServerConfig::find(std::string api_name, std::string target) const {
// 	for (size_t i = 0; i < lnum; i++)
// 	{
// 		for (size_t j = 0; j < m_locations[i].size(); j++)
// 		// if (checkExpression(i, api_name))
// 		if (m_locations[i][j].getEndPoint() == api_name)
// 			return m_locations[i][j].find(target);
// 	}
// 	std::cerr << "check api name" << std::endl;
// 	Exception::handleInvalidAccess();
// 	return "";
// }

// std::string	ServerConfig::find(std::string target) const {
// 	return AConfig::find(target);
// }
// unsigned int	ServerConfig::getter(std::string target) const {
// 	return AConfig::getter(target);
// }

// std::string	ServerConfig::getServerName() const {
// 	return m_ser
// }

// int	ServerConfig::getPort() const {
// 	std::string	portStr;
// 	int			port;

// 	port = 0;
// 	portStr = this->AConfig::find("listen");
// 	for (size_t i = 0; i < portStr.size(); i++)
// 		port = port * 10 + (portStr[i] - '0');
// 	return port;
// }

void	ServerConfig::checkValidConfigs() {
	std::set<std::string>	api_names;
	std::string				name;

	AConfig::checkValidConfigs();
	for (size_t i = 0; i < lnum; i++)
	{
		api_names.clear();
		for (size_t	j = 0; j < m_locations[i].size(); j++)
		{
			name = m_locations[i][j].getEndPoint();
			if(api_names.find(name) == api_names.end())
				api_names.insert(name);
			else
				Exception::handleInvalidFile();
			m_locations[i][j].checkValidConfigs();
		}
	}
}

bool	ServerConfig::getUriSetting(std::string &api_point, LocationConfig &obj) const{
	size_t size;

	size = 0;
	if (api_point.size() && api_point[api_point.size() - 1] == '?')
		api_point = api_point.substr(0, api_point.size() - 1);
	for (int i = 0; i < lnum; i++)
	{
		for (size_t j = 0; j < m_locations[i].size(); j++)
		{
			// std::cout << "location setting check priority : " << i << " " << m_locations[i][j].getEndPoint() << " " << api_point << std::endl;
			if (m_locations[i][j].match(api_point))
			{
				if (size < m_locations[i][j].getEndPoint().size())
				{
					size = m_locations[i][j].getEndPoint().size();
					obj = m_locations[i][j];
				}
			}
		}
		if (size)
			break ;
	}
	return size != 0;
}