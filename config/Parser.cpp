#include "Parser.hpp"

Parser::Parser(std::string file) {
	getFile(file);
}

Parser::~Parser() { }

std::string	Parser::fileToStr(std::string file)
{
	std::ifstream		file_in;
	std::stringstream	ss;
	struct stat			sb;

	if (stat(file.c_str(), &sb) == -1)
	{
		std::cerr << "DEBUG: no File " << file << std::endl;
		Exception::handleInvalidFile(errno);
	}
	file_in.open(file.c_str());
	if (!file_in)
	{
		std::cerr << "DEBUG: No file to parse " << file << std::endl;
		Exception::handleInvalidFile();
	}
	ss << file_in.rdbuf();
	file_in.close();
	return ss.str();
}

void	Parser::getFile(std::string file)
{
	std::deque<std::string>	frags;
	std::string				line;

	line = fileToStr(file);
	setFragments(frags, line);
	m_frags.insert(m_frags.begin(), frags.begin(), frags.end());
}

void	Parser::setFragments(std::deque<std::string> &frags, std::string &line) {
	size_t					prev, end;

	prev = 0;
	end = 0;
	while (end < line.size())
		splitBySpaces(frags, line, prev, end);
	if (prev != end)
		frags.push_back(line.substr(prev, end));
}

size_t	Parser::splitBySpaces(std::deque<std::string> &frags, std::string &line, size_t &prev, size_t &end)
{
	if (line[end] == '#')
	{
		if (line.find('\n', prev) == std::string::npos)
			end = line.size();
		else
			end = line.find('\n', prev);
		prev = end + 1;
	}
	else if (line[end] == '\t' || line[end] == '\n' || line[end] == ' ')
	{
		if (prev != end)
			frags.push_back(line.substr(prev, end - prev));
		prev = end + 1;
	}
	return ++end;
}

void	Parser::getFiles(std::deque<std::string> &frag)
{
	std::deque<std::string>::iterator	it;
	std::deque<std::string>				ret;
	size_t								size;

	size = frag.size();
	while (size--)
	{
		ret = handleWildcard(frag.front());
		frag.pop_front();
		frag.insert(frag.end(), ret.begin(), ret.end());
		ret.clear();
	}
	for (it = frag.begin(); it != frag.end(); it++)
		getFile(*it);
}

void	Parser::getFiles(std::string &files)
{
	std::deque<std::string>::iterator	it;
	std::deque<std::string>				frags, ret;
	size_t								prev, end, size;

	prev = 0; end = 0;
	while (end < files.size())
		splitBySpaces(frags, files, prev, end);
	if (prev != end)
		frags.push_back(files.substr(prev, end));
	size = frags.size();
	while (size--)
	{
		ret = handleWildcard(frags.front());
		frags.pop_front();
		frags.insert(frags.end(), ret.begin(), ret.end());
	}
	for (it = frags.begin(); it != frags.end(); it++)
		getFile(*it);
}

void	Parser::printFragments() const {
	std::deque<std::string>::const_iterator	it;

	std::cout << "<----------------DEBUG---------------->" << std::endl;
	std::cout << "<----------------FRAGS---------------->" << std::endl;
	for (it = m_frags.begin(); it != m_frags.end(); it++) {
		std::cout << "frags: " << *it << std::endl;
	}
	std::cout << "<----------------PRINT END---------------->" << std::endl;
}

void	Parser::handleDirectives(AConfig &obj) {
	ServerConfig	server;

	while (!m_frags.empty())
	{
		if (m_frags.front() == "http")
			handleBlockDirectives(obj);
		else if (m_frags.front() == "events")
			handleBlockDirectives(obj);
		else if (m_frags.front() == "server")
		{
			handleBlockDirectives(server);
			obj.setConfig(server);
			server.clear();
		}
		else if (m_frags.front() == "types")
			handleTypeDirectives(obj);
		else
			handleSimpleDirectives(obj);
	}
}

bool	Parser::isLocationScope(AConfig& obj) const {
	if (dynamic_cast<LocationConfig*>(const_cast<AConfig*>(&obj)))
		return true;
	return false;
}

size_t	Parser::findValueDeque(const std::string& val) const {
	size_t	pos;
	for (pos = 0; pos < m_frags.size(); pos++)
	{
		if (m_frags[pos] == val)
			return pos;
	}
	return size_t(-1);
}

void	Parser::preCheckBlock(AConfig& obj) {
	size_t	pos;

	pos = findValueDeque("{");
	obj.setApiPoint(m_frags, pos);
	m_frags.erase(m_frags.begin(), m_frags.begin() + pos + 1);
}

void	Parser::handleTypeDirectives(AConfig &obj) {
	std::deque<std::string>	deq;
	size_t	pos;

	preCheckBlock(obj);
	while (!m_frags.empty() && m_frags.front() != "}")
	{
		if ((pos = findEndCharsetFromStrs(';')) == (size_t)-1)
		{
			std::cerr << "DEBUG: invalid simple2. " << std::endl;
			Exception::handleInvalidFile();
		}
		m_frags[pos].erase(m_frags[pos].size() - 1);
		for (size_t i = 1; i <= pos; i++)
			obj.addTypeVal(m_frags[i], m_frags[0]);
		m_frags.erase(m_frags.begin(), m_frags.begin() + pos + 1);
	}
	postCheckBlock();
}

void	Parser::handleBlockDirectives(AConfig &obj) {
	ServerConfig	server;
	LocationConfig	location;

	preCheckBlock(obj);
	while (!m_frags.empty() && m_frags.front() != "}")
	{
		if (m_frags.front() == "server")
		{	
			handleBlockDirectives(server);
			obj.setConfig(server);
			server.clear();
		}
		else if (m_frags.front() == "location")
		{
			handleBlockDirectives(location);
			obj.setConfig(location);
			location.clear();
		}
		else if (m_frags.front() == "types")
			handleTypeDirectives(obj);
		else
			handleSimpleDirectives(obj);
	}
	postCheckBlock();
}

void	Parser::bringKeyAndValue(std::string &key, std::string &val, size_t pos) {
	val = "";
	for (size_t i = 1; i <= pos; i++) 
			val += m_frags[i] + " ";
	key = m_frags[0];
	m_frags.erase(m_frags.begin(), m_frags.begin() + pos + 1);
}

void	Parser::handleSimpleDirectives(AConfig &obj) {
	std::string	key, val;
	std::deque<std::string>	frag;
	size_t	pos;

	if ((pos = findEndCharsetFromStrs(';')) == (size_t)-1)
	{
		std::cerr << "DEBUG: invalid simple2. " << std::endl;
		Exception::handleInvalidFile();
	}
	m_frags[pos].erase(m_frags[pos].size() - 1);
	frag.insert(frag.end(), m_frags.begin(), m_frags.begin() + pos + 1);
	m_frags.erase(m_frags.begin(), m_frags.begin() + pos + 1);
	if (frag[0] == "include")
	{
		frag.pop_front();
		getFiles(frag);
	}
	else
		obj.addVal(frag);
}

size_t	Parser::findEndCharsetFromStrs(char end) {
	size_t	pos;

	for (pos = 0; pos < m_frags.size(); pos++) {
		if (m_frags[pos][m_frags[pos].size() - 1] == end)
			return pos;
	}
	return (size_t) -1;
}


void	Parser::postCheckBlock() {
	if (m_frags.empty() || m_frags.front() != "}")
	{
		std::cerr << "DEBUG: no closed block " << std::endl;
		Exception::handleInvalidFile();
	}
	// std::cout << "scope: " << scope << " is blocked." << std::endl;
	m_frags.pop_front();
}

std::string findParentDirectory(std::string& file)
{
	std::string	ret;
	size_t		dir_pos, asterisk;

	dir_pos = file.find('/');
	asterisk = file.find('*');
	while (dir_pos != std::string::npos)
	{
		//dir_pos가 없거나
		if (file.find('/', dir_pos) < asterisk)
			break ;
		dir_pos = file.find('/', dir_pos);
	}
	if (dir_pos == std::string::npos)
		ret = "./";
	else
	{
		ret = file.substr(dir_pos);
		file.erase(dir_pos);
	}
	return ret;
}

bool	hasFolder(std::string file) {
	return file.find('/') != std::string::npos;
}

std::deque<std::string>	Parser::handleWildcard(std::string file)
{
	struct dirent			*entry;
	DIR						*dir;
	std::deque<std::string>	ret;
	std::string				parent_dir;
	size_t					pos;
	bool					is_dir;

	pos = file.find('*');
	if (pos == std::string::npos)
	{
		ret.push_back(file);
		return ret;
	}
	parent_dir = findParentDirectory(file);
	is_dir = hasFolder(file);
	if (!(dir = opendir(file.substr(pos).c_str())))
		Exception::handleInvalidFile(errno);
	while ((entry = readdir(dir)))
	{
		if (!is_dir && entry->d_name == file)
			ret.push_back(parent_dir + file);
	}
	closedir(dir);
	return ret;
}