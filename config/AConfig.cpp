
#include "AConfig.hpp"

AConfig::AConfig(/* args */): m_configs() { }
// AConfig::AConfig(/* args */): m_configs(), m_default(), m_scope(e_main) { }
// AConfig::AConfig(t_scope scope): m_configs(), m_default(), m_scope(scope) { }

AConfig::~AConfig() { }

// AConfig::AConfig(const AConfig &srcs) {
// 	*this = srcs;
// }


AConfig&	AConfig::operator=(const AConfig& obj) {
	std::map<std::string, std::string>::iterator it;

	if (this == &obj)
		return *this;
	// m_scope = obj.getScope();
	m_configs = obj.m_configs;
	m_types = obj.m_types;
	m_errors = obj.m_errors;
	m_methods = obj.m_methods;
	m_index = obj.m_index;
	m_redirects = obj.m_redirects;
	m_ports = obj.m_ports;
	m_server_names = obj.m_server_names;
	m_ips = obj.m_ips;
	// m_default = obj.getDefaultConfigs();
	return *this;
}


void	AConfig::clear() {
	m_configs.clear();
	m_types.clear();
	m_errors.clear();
	m_methods.clear();
	m_index.clear();
	m_redirects.clear();
	m_ips.clear();
	// m_configs_int.clear();
	m_ports.clear();
	m_server_names.clear();
}

// std::map<std::string, std::string>	AConfig::getDefaultConfigs() const {
// 	return m_default;
// }

std::map<std::string, std::string>	AConfig::getConfigs() const {
	return m_configs;
}

std::map<std::string, std::string>	AConfig::getErrors() const {
	return m_errors;
}



std::map<std::string, std::string>	AConfig::getTypes() const {
	return m_types;
}

std::vector<std::string>			AConfig::getIndex() const {
	return m_index;
}

std::vector<std::string>			AConfig::getMethods() const {
	return m_methods;
}

std::vector<std::string>			AConfig::getRedirects() const {
	return m_redirects;
}

void	AConfig::addConfigIfNotExist(std::map<std::string, std::string> obj) {
	std::map<std::string, std::string>::iterator	it;
	
	for (it = obj.begin(); it != obj.end(); it++)
	{
		if (m_configs.find(it->first) != m_configs.end())
			continue ;
		if (getInstance()->findTarget(it->first) != size_t(-1))
			m_configs[it->first] = it->second;
	}
	
}

void	AConfig::replaceConfig(const AConfig &obj) {
	*this = obj;
}

void	AConfig::addTypesIfNotExist(std::map<std::string, std::string> obj) {
	std::map<std::string, std::string>::const_iterator	it;
	for (it = obj.cbegin(); it != obj.cend(); it++)
	{
		if (m_types.find(it->first) != m_types.end())
			continue ;
		m_types[it->first] = it->second;
	}
}

void	AConfig::addErrorsIfNotExist(std::map<std::string, std::string> obj) {
	std::map<std::string, std::string>::const_iterator	it;

	for (it = obj.cbegin(); it != obj.cend(); it++)
	{
		if (m_errors.find(it->first) != m_errors.end())
			continue ;
		m_errors[it->first] = it->second;
	}
}

void	AConfig::setApiPoint(const std::deque<std::string> &deq, size_t pos) {
	(void) deq;
	if (pos != 1)
	{
		std::cerr << "DEBUG: invalid SET API." << std::endl;
		Exception::handleInvalidFile();
	}
}

size_t	AConfig::getConfigSize() const {
	return m_configs.size();
}

void	AConfig::addTypeVal(const std::string key, const std::string val) {
	m_types[key] = val;
}

void	AConfig::addDefaultVal(std::deque<std::string> &deq) {
	std::stringstream	ss;

	if (deq[0] == "error_page")	{
		setErrorPage(deq);
	}
	else if (deq[0] == "listen")
		setAddress(deq);
	else if (deq[0] == "server_name")
		setServerNames(deq);
	else if (deq[0] == "limit_except")
		setMethod(deq);
	else if (deq[0] == "index")
		setIndex(deq);
	else if (deq[0] == "return") 
		setRedirects(deq);
	// else if (deq[0] == "try_files")
		// setTryFiles(deq);
	m_configs[deq[0]] = deq[1];
}

void	AConfig::addVal(std::deque<std::string> &deq) {
	SyntaxChecker::checkSyntax(deq);
	getInstance()->find(deq[0]);
	if (deq[0] == "error_page")
		setErrorPage(deq);
	else if (deq[0] == "listen")
		setAddress(deq);
	else if (deq[0] == "server_name")
		setServerNames(deq);
	else if (deq[0] == "index")
		setIndex(deq);
	else if (deq[0] == "limit_except")
		setMethod(deq);
	else if (deq[0] == "return") 
		setRedirects(deq);
	else
		m_configs[deq[0]] = deq[1];
}

void	AConfig::setPort(const std::string &port_str)
{
	unsigned short 	port;

	port = SyntaxChecker::checkValidNum(port_str, 65535);
	m_ports.push_back(port);
}

void	AConfig::setAddress(const std::deque<std::string> &deq)
{
	std::string	val;
	size_t		pos;

	for (size_t i = 1; i < deq.size(); i++)
	{
		if ((pos = deq[i].find(':')) != (size_t) -1)
		{
			setIP(deq[i].substr(0, pos));
			setPort(deq[i].substr(pos + 1));
		}
		else if (deq[i] == "localhost" || deq[i].find('.') != (size_t) -1)
		{
			setIP(deq[i]);
			setPort(val);
		}
		else
		{
			setIP("*");
			setPort(deq[i]);
		}
	}
}

void	AConfig::setIP(const std::string &ip_str)
{
	unsigned int	ip;

	if (ip_str == "localhost")
		ip = SyntaxChecker::makeIP("127.0.0.1");
	else
		ip = SyntaxChecker::makeIP(ip_str);
	m_ips.push_back(ip);
}

void	AConfig::setErrorPage(const std::deque<std::string> &deq)
{
	// if (deq[1] == "-")
		// return ;
	for (size_t i = 1; i < deq.size() - 1; i++)
		m_errors[deq[i]] = deq.back();
}
void	AConfig::setIndex(const std::deque<std::string> &deq)
{
	for (size_t i = 1; i < deq.size(); i++)
		m_index.push_back(deq[i]);
}
void	AConfig::setServerNames(const std::deque<std::string> &deq)
{
	for (size_t i = 1; i < deq.size(); i++)
		m_server_names.push_back(deq[i]);
}
void	AConfig::setMethod(const std::deque<std::string> &deq)
{
	for (size_t i = 1; i < deq.size(); i++)
		m_methods.push_back(deq[i]);
}

void	AConfig::setRedirects(const std::deque<std::string> &deq)
{
	for (size_t i = 1; i < deq.size(); i++)
		m_redirects.push_back(deq[i]);
}

void	AConfig::printConfigs() const {
	std::map<std::string, std::string>::const_iterator	it;

	std::cout << "<----------------DEBUG---------------->" << std::endl;
	std::cout << "<----------------FILECONFIG---------------->" << std::endl;
	for (it = m_configs.begin(); it != m_configs.end(); it++)
	{
		// std::cout << "key: " << it->first << " val: " << it->second << std::endl;
		std::cout << "File: " << it->first << " " << it->second << std::endl;
	}
	// std::cout << "<----------------DEFAULT---------------->" << std::endl;
	// for (it = m_default.begin(); it != m_default.end(); it++)
	// {
	// 	std::cout << "Default: " << it->first << " val: " << it->second << std::endl;
	// }
	// std::cout << "<----------------PRINT END---------------->" << std::endl;
}


void	AConfig::printTypes() const {
	std::map<std::string, std::string>::const_iterator	it;

	std::cout << "<----------------DEBUG---------------->" << std::endl;
	std::cout << "<----------------TypeCONFIG---------------->" << std::endl;
	for (it = m_types.begin(); it != m_types.end(); it++)
	{
		std::cout << "Tyoe: " << it->first << " " << it->second << std::endl;
	}
}

void	AConfig::printErrors() const {
	std::map<std::string, std::string>::const_iterator	it;

	std::cout << "<----------------DEBUG---------------->" << std::endl;
	std::cout << "<----------------ErrorCONFIG---------------->" << std::endl;
	for (it = m_errors.begin(); it != m_errors.end(); it++)	{
		std::cout << "Error: " << it->first << " " << it->second << std::endl;
	}
}

void	AConfig::checkValidConfigs() {
	std::map<std::string, std::string>::const_iterator	it;

	for (it = m_configs.cbegin(); it != m_configs.cend(); it++)
		getInstance()->find(it->first);
}

size_t	AConfig::findTarget(std::string target) const {
	std::map<std::string, std::string>::const_iterator	it;
	size_t												id;

	id = 0;
	for (it = m_configs.begin(); it != m_configs.end(); it++)
	{
		if (it->first == target)
			return id;
		id++;
	}		
	return (size_t) -1;
}

size_t	AConfig::findTypeTarget(std::string target) const {
	std::map<std::string, std::string>::const_iterator	it;
	size_t												id;

	id = 0;
	for (it = m_types.begin(); it != m_types.end(); it++)
	{
		if (it->first == target)
			return id;
		id++;
	}		
	return (size_t) -1;
}

size_t	AConfig::findMethodTarget(std::string target) const {
	std::vector<std::string>::const_iterator	it;
	size_t										id;

	id = 0;
	for (it = m_methods.begin(); it != m_methods.end(); it++)
	{
		if (*it == target)
			return id;
		id++;
	}		
	return (size_t) -1;
}

std::string	AConfig::findType(std::string target) const {
	std::map<std::string, std::string>::const_iterator	it;

	if ((it = m_types.find(target)) == m_types.cend())
		Exception::handleInvalidAccess();
	return it->second;
}

std::string	AConfig::findErrorPage(std::string target) const {
	std::map<std::string, std::string>::const_iterator	it;

	if ((it = m_errors.find(target)) == m_errors.cend())
		Exception::handleInvalidAccess();
	return it->second;
}

std::string	AConfig::find(std::string target) const {
	std::map<std::string, std::string>::const_iterator	it;

	if ((it = m_configs.find(target)) == m_configs.cend())
	{
		std::cerr << "the context is " << target << std::endl;
		Exception::handleInvalidAccess();
	}
	return it->second;
}

unsigned int	AConfig::getter(std::string target) const {
	double		value;
	size_t		coeff;
	char		*ptr;
	std::string	val_str;

	coeff = 1;
	val_str = find(target);
	value = strtod(val_str.c_str(), &ptr);
	if (*(ptr + 1) || value > 1000 || value < 0)
		Exception::handleInvalidAccess();
	if (*ptr == 'K' || *ptr == 'k')
		coeff = 1000;
	else if (*ptr == 'm' || *ptr == 'M')
		coeff = 60;
	else if (*ptr == 'h')
		coeff = 3600;
	return coeff * value;
}

bool	AConfig::hasAddress(unsigned int ip, unsigned short port) const {
	for (size_t	i = 0; i < m_ports.size(); i++)
	{
		if (m_ports[i] == port && (m_ips[i] == ip || !m_ips[i]))
			return true;
	}
	return false;
}

size_t	AConfig::checkServerName(std::string &server_name) const {
	size_t	j;

	for (size_t i = 0; i < m_server_names.size(); i++)
	{
		if (server_name == m_server_names[i] || server_name == "." + m_server_names[i])
			return 1;
		else if (m_server_names[i].find('*') == std::string::npos || m_server_names[i].size() > server_name.size())
			continue ;
		else if (m_server_names[i][0] == '*')
		{
			for (j = 0; j < m_server_names[i].size(); j++)
			{
				if (m_server_names[i][m_server_names[i].size() - 1 - j] != server_name[server_name.size() - j - 1] && j != m_server_names[i].size() - 1)
					break ;					
			}
			if (j == m_server_names[i].size())
				return 2;
		}
		else
		{
			for (j = 0; j < m_server_names[i].size(); j++)
			{
				if (m_server_names[i][j] != server_name[j] && j != m_server_names[i].size() - 1)
					break ;					
			}
			if (j == m_server_names[i].size())
				return 3;		
		}
	}
	return 4;
}

std::vector<unsigned short>	AConfig::getPort() const{
	return m_ports;
}

std::vector<unsigned int>	AConfig::getIPs() const{
	return m_ips;
}


void	AConfig::addIndexIfNotExist(std::vector<std::string> obj) {
	if (m_index.size())
		return;
	m_index = obj;
}

void	AConfig::addMethodsIfNotExist(std::vector<std::string> obj) {
	if (m_methods.size())
		return;
	m_methods = obj;
}

void	AConfig::addRedirectsIfNotExist(std::vector<std::string> obj) {
	if (m_redirects.size())
		return ;
	m_redirects = obj;
}

void	AConfig::addAddressIfNotExist(std::vector<unsigned int> ip, std::vector<unsigned short> ports) {
	if (m_ips.size())
		return ;
	m_ips = ip;
	m_ports = ports;
}
void	AConfig::printmErrors() const {
	std::map<std::string, std::string>::const_iterator	it = m_errors.cbegin();

	for (; it != m_errors.cend(); it++)
		std::cout << it->first << " " << it->second << std::endl;
	std::cout << std::endl;
}