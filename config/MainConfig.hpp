#ifndef MAINCONFIG_HPP
# define MAINCONFIG_HPP
# include <iostream>
# include <deque>
# include <netdb.h>
# include <vector>
# include "AConfig.hpp"
# include "ServerConfig.hpp"
# include "FileParser.hpp"
# include "Parser.hpp"
# include "../exception/Exception.hpp"
# define http_conf "utils/conf.d/requirements/http.nginx_keys"

class MainConfig : public AConfig {
	private:
		std::vector<ServerConfig>	m_servers;
		static AConfig				*mp_default;
		MainConfig();

	public:
		MainConfig(std::string file);
		MainConfig& operator=(const MainConfig& objs);
		~MainConfig();
		virtual AConfig*		getInstance();
		void					setServer(const AConfig& obj);
		virtual void			setConfig(const AConfig& obj);
		virtual void			updateDefaultConfig();
		virtual void			checkValidConfigs();
		virtual void			inheritConfig(const AConfig &obj);
		virtual void			printConfigs() const;
		static void				deleteConfig();		
		const std::vector<ServerConfig>	getServerConfigs() const;
		std::vector<unsigned short>	getPorts() const;
		std::vector<unsigned int>	getIPs() const;
		ServerConfig			findServer(std::string &server_name, unsigned int ip, unsigned short port) const;
};

#endif