#ifndef ACONFIG_HPP
# define ACONFIG_HPP
#include <iostream>
#include <sstream>
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <set>
#include "../exception/Exception.hpp"
#include "../checker/SyntaxChecker.hpp"
#include "../checker/Checker.hpp"

struct cmp_l {
	bool operator()(std::string &s1, std::string &s2) {
		return s1.size() < s2.size();
	}
};

class AConfig
{
	private:
		std::map<std::string, std::string>  m_configs;
		std::map<std::string, std::string>  m_types;
		std::map<std::string, std::string>	m_errors;
		std::vector<std::string> 			m_methods;
		std::vector<std::string> 			m_index;
		std::vector<std::string> 			m_redirects;
		std::vector<std::string>			m_server_names;
		std::vector<unsigned int>			m_ips;
		std::vector<unsigned short>			m_ports;
		AConfig&							operator=(const AConfig& objs);

	public:
		AConfig();
		virtual ~AConfig();
		std::map<std::string, std::string>  getConfigs() const;
		std::map<std::string, std::string>  getTypes() const;
		std::map<std::string, std::string>  getErrors() const;
		std::vector<std::string>  			getIndex() const;
		std::vector<std::string>  			getRedirects() const;
		std::vector<std::string>  			getMethods() const;
		// std::vector<std::string>  			getTryFiles() const;
		std::vector<unsigned short>			getPort() const;
		std::vector<unsigned int>			getIPs() const;
		virtual void						setConfig(const AConfig& obj) = 0;
		virtual void						inheritConfig(const AConfig& obj) = 0;
		virtual void						setApiPoint(const std::deque<std::string> &deq, size_t pos);
		virtual void						updateDefaultConfig() = 0;
		virtual void						clear();
		virtual size_t						findTarget(std::string target) const;
		virtual size_t						findMethodTarget(std::string target) const;
		virtual size_t						findTypeTarget(std::string target) const;
		virtual std::string					findErrorPage(std::string target) const;
		virtual std::string					findType(std::string target) const;
		size_t								getConfigSize() const;
		std::string							find(std::string target) const;
		virtual void						checkValidConfigs();
		virtual void						printConfigs() const;
		bool								checkValueIsNumber(std::string &key, std::string &val);
		unsigned int						getter(std::string target) const;
		bool								checkValidValue(std::string &key, std::string &val);
		bool								hasAddress(unsigned int ip, unsigned short port) const;
		size_t								checkServerName(std::string &server_name) const;
		virtual AConfig						*getInstance() = 0;
		void								printErrors() const;
		void								printmErrors() const;
		void								setPort(const std::string &port);
		void								setIP(const std::string &ip);
		void								setAddress(const std::string &ip, const std::string &port);
		void								setErrorPage(const std::deque<std::string> &deq);
		void								setIndex(const std::deque<std::string> &deq);
		void								setServerNames(const std::deque<std::string> &deq);
		void								setMethod(const std::deque<std::string> &deq);
		void								setAddress(const std::deque<std::string> &deq);
		void								setRedirects(const std::deque<std::string> &deq);
		void								setIPWithPort(std::string ip_str, std::string port);
		void								addDefaultVal(std::deque<std::string> &deq);
		void								printTypes() const;
		void								replaceConfig(const AConfig& obj);
		void								addConfigIfNotExist(std::map<std::string, std::string> obj);
		void								addTypesIfNotExist(std::map<std::string, std::string> obj);
		void								addErrorsIfNotExist(std::map<std::string, std::string> obj);
		void								addIndexIfNotExist(std::vector<std::string> obj);
		void								addMethodsIfNotExist(std::vector<std::string> obj);
		void								addRedirectsIfNotExist(std::vector<std::string> obj);
		void								addAddressIfNotExist(std::vector<unsigned int> ip, std::vector<unsigned short> port);
		void								addType(std::string& key, std::string& val);
		void								addTypeVal(const std::string key, const std::string val);
		void								addVal(std::string& key, std::string& val);
		void								addVal(const std::string key, const std::string val);
		void								addVal(std::deque<std::string>& deque);
};

#endif