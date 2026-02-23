#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
# include <vector>
# include "LocationConfig.hpp"
# include "FileParser.hpp"
# define server_conf "utils/conf.d/requirements/server.nginx_keys"
# define lnum 3

class MainConfig;

class ServerConfig : public AConfig
{
	private:
		static AConfig	*mp_default;
		std::vector<LocationConfig> m_locations[lnum];
		ServerConfig(std::string file);

	public:
		ServerConfig();
		~ServerConfig();
		ServerConfig&	operator=(const ServerConfig& objs);
		void			setLocation(size_t priority, const std::vector<LocationConfig> &obj);
		virtual void	setConfig(const AConfig& obj);
		virtual void	clear();
		size_t			getSizeOfLocations() const;
		std::string		getServerName() const;
		virtual void	inheritConfig(const AConfig& obj);
		void			addNewLocation(size_t priority, const LocationConfig& obj);
		void			addNewLocation(std::string& api_point, std::string priority);
		virtual void	updateDefaultConfig();
		virtual void	printConfigs() const;
		virtual void	checkValidConfigs();
		virtual AConfig	*getInstance();
		std::string		getErrorPage(const std::string &code) const;
		static void		deleteConfig();
		bool			getUriSetting(std::string &api_point, LocationConfig &obj) const;
		void			clearLocation();
};
# include "MainConfig.hpp"

#endif