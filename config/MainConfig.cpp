#include "MainConfig.hpp"

AConfig	*MainConfig::mp_default = new MainConfig();
MainConfig::MainConfig(): AConfig(), m_servers() {
	FileParser								parser;

	parser.getFile(http_conf);
	parser.setDefault(*this);
}

MainConfig::MainConfig(std::string file): AConfig(), m_servers()
{
	Parser	parser(file);

	parser.handleDirectives(*this);
	for (size_t i = 0; i < m_servers.size(); i++)
		m_servers[i].inheritConfig(*this);
	updateDefaultConfig();
}

void	MainConfig::updateDefaultConfig() {
	AConfig								*ptr;

	ptr = getInstance();
	addConfigIfNotExist(ptr->getConfigs());
	addErrorsIfNotExist(ptr->getErrors());
	addTypesIfNotExist(ptr->getTypes());
	addIndexIfNotExist(ptr->getIndex());
	addRedirectsIfNotExist(ptr->getRedirects());
	for (size_t i = 0; i < m_servers.size(); i++)
		m_servers[i].updateDefaultConfig();
}


void	MainConfig::printConfigs() const {
	std::cout << "<----------------MAINCONFIG---------------->" << std::endl;
	this->AConfig::printConfigs();
	this->AConfig::printTypes();
	this->AConfig::printErrors();
	for (size_t i = 0; i < m_servers.size(); i++)
	{
		std::cout << "<----------------SERVERCONFIG " << i << " ---------------->" << std::endl;
		m_servers[i].printConfigs();
		std::cout << "<----------------SERVERType " << i << " ---------------->" << std::endl;
		m_servers[i].printTypes();
		std::cout << "<----------------SERVERErrors " << i << " ---------------->" << std::endl;
		m_servers[i].printErrors();
	}
}

MainConfig::~MainConfig() {}

void	MainConfig::deleteConfig() {
	if (mp_default)
		delete mp_default;
}
AConfig	*MainConfig::getInstance()
{
	if (mp_default == NULL)
		mp_default = new MainConfig();
	return mp_default;
}

MainConfig&	MainConfig::operator=(const MainConfig& obj)
{
	if (this == &obj)
		return *this;
	m_servers.reserve(obj.m_servers.size());
	m_servers = obj.m_servers;
	// AConfig::setScope(obj.getScope());
	AConfig::addConfigIfNotExist(obj.getConfigs());
	return *this;
}

std::vector<unsigned short>	MainConfig::getPorts() const{
	std::vector<unsigned short>	ret, ports;

	for (size_t i = 0; i < m_servers.size(); i++)
	{
		ports = m_servers[i].getPort();
		ret.insert(ret.end(), ports.begin(), ports.end());
	}
	return ret;
}

std::vector<unsigned int>	MainConfig::getIPs() const{
	std::vector<unsigned int>	ret, ports;

	for (size_t i = 0; i < m_servers.size(); i++)
	{
		ports = m_servers[i].getIPs();
		ret.insert(ret.end(), ports.begin(), ports.end());
	}
	return ret;
}

void	MainConfig::setConfig(const AConfig& obj) {
	MainConfig			*is_http;
	ServerConfig	*is_server;
	
	is_http = dynamic_cast<MainConfig*>(&(const_cast<AConfig&>(obj)));
	is_server = dynamic_cast<ServerConfig*>(&(const_cast<AConfig&>(obj)));
	if (is_http)
		*this = *is_http;
	else if (is_server)
		m_servers.push_back(*is_server);
	else
		Exception::handleInvalidFile();
}

void	MainConfig::inheritConfig(const AConfig& obj) {
	ServerConfig	*is_server;
	
	is_server = dynamic_cast<ServerConfig*>(&(const_cast<AConfig&>(obj)));
	if (is_server)
		is_server->inheritConfig(*this);
	else
		Exception::handleInvalidFile();
}

void	MainConfig::checkValidConfigs() {
	AConfig::checkValidConfigs();
	for (size_t i = 0; i < m_servers.size(); i++)
		m_servers[i].checkValidConfigs();
}

ServerConfig	MainConfig::findServer(std::string &server_name, unsigned int ip, unsigned short port) const {
	ServerConfig	ins, *ptr;
	size_t			priority, id;

	priority = 5;
	id = (size_t) -1;
	ptr = dynamic_cast<ServerConfig *>(ins.getInstance());
	for (size_t i = 0; i < m_servers.size(); i++)
	{
		if (m_servers[i].hasAddress(ip, port) && priority > m_servers[i].checkServerName(server_name))
		{
			priority = m_servers[i].checkServerName(server_name);
			id = i;
		}
		if (priority == 1)
			break ;
	}
	if (id == (size_t) -1)
		Exception::handleInvalidFile();
	else if (id == (size_t) -1)
		return *ptr;
	return m_servers[id];
}
const std::vector<ServerConfig>	MainConfig::getServerConfigs() const {
	return m_servers;
}
