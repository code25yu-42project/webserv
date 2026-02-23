#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP
#include "AConfig.hpp"
# include "FileParser.hpp"
# define location_conf "utils/conf.d/requirements/location.nginx_keys"

typedef enum e_priority{
	e_exactly, // =
	e_preferential_prefix_match, // ^~
	// e_regex_match_with_sensitive_case, // ~
	// e_regex_match_with_insensitive_case, // ~*
	e_prefix_match // ""
}	t_priority;

class ServerConfig;

class LocationConfig : public AConfig
{
	private:
		std::string m_api_point;
		t_priority	m_priority;
		static AConfig	*mp_default;
		LocationConfig(std::string file);

	public:
		LocationConfig();
		LocationConfig(std::string &api_point, std::string priority);
		LocationConfig&	operator=(const LocationConfig& obj);
		bool			operator==(const LocationConfig& obj);
		~LocationConfig();
		t_priority		getPriority() const;
		virtual void	updateDefaultConfig();
		void			setPriority(std::string priority);
		void			setConfig(const AConfig& obj);
		std::string		getEndPoint() const;
		virtual void			checkValidConfigs();
		// std::string		find(std::string target) const;
		void			setMethod(const std::deque<std::string> &deq, size_t pos);
		virtual void	setApiPoint(const std::deque<std::string> &deq, size_t pos);
		// unsigned int	getter(std::string target) const;
		virtual void	inheritConfig(const AConfig& obj);
		virtual void	printConfigs() const;
		static void				deleteConfig();
		// virtual void	updateDefaultSettings();
		virtual AConfig	*getInstance();
		bool			match(const std::string &api_point) const;
		std::string		getContentType(const std::string &target) const;
		std::string		getErrorPage(const std::string &code) const;
		bool			isValidMethod(const std::string &method) const;
		bool			hasRedirect(std::string &redirect, std::string &status) const;
		bool			hasProxy(std::string &pass) const;
		bool			hasCgi(const std::string &api, std::string &script_name, std::string &path_info) const;
		bool			hasAutoIndex(std::string api_point, std::string &path, std::string &status) const;
		bool			hasIndex(std::string api_point, std::string &path, std::string &status) const;
};
# include "ServerConfig.hpp"

#endif
