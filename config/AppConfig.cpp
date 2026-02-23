#include "AppConfig.hpp"

AppConfig	*AppConfig::mp_config = NULL;

AppConfig::AppConfig(std::string file_cf): m_config(file_cf) { }

AppConfig::~AppConfig() {
	if (mp_config)
		delete mp_config;
	MainConfig::deleteConfig();
	ServerConfig::deleteConfig();
	LocationConfig::deleteConfig();
}

AppConfig	*AppConfig::getInstance(std::string file_cf) {
	if (!mp_config)
		mp_config = new AppConfig(file_cf);
	return mp_config;
}

std::vector<unsigned short>	AppConfig::getPorts() const {
		return m_config.getPorts();
}

std::vector<unsigned int>	AppConfig::getIPs() const {
		return m_config.getIPs();
}

std::string	AppConfig::find(std::string target) const {
	return m_config.find(target);
}

unsigned int	AppConfig::getter(std::string target) const {
	return m_config.getter(target);
}

const ServerConfig	AppConfig::getServerConfig(std::string &server_name, unsigned int ip, unsigned short port) const {
	return m_config.findServer(server_name, ip, port);
}

const std::vector<ServerConfig>	AppConfig::getServerConfigs() const {
	return m_config.getServerConfigs();
}