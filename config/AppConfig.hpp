#ifndef APPCONFIG_HPP
# define APPCONFIG_HPP
# include <iostream>
# include <string>
# include "Parser.hpp"
# include "MainConfig.hpp"

class AppConfig
{
	private:
		AppConfig(std::string file_cf);
		~AppConfig();
		static AppConfig	*mp_config;
		MainConfig			m_config;
	public:
		static AppConfig				*getInstance(std::string file_cf);
		const ServerConfig				getServerConfig(std::string &server_name, unsigned int ip, unsigned short port) const;
		const std::vector<ServerConfig>	getServerConfigs() const;
		unsigned int					getter(std::string target) const;
		std::string						find(std::string target) const;
		std::vector<unsigned short>		getPorts() const;
		std::vector<unsigned int>		getIPs() const;
};
#endif