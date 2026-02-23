#include "FileParser.hpp"
FileParser::FileParser():m_frags() {}

FileParser::~FileParser() { }

size_t	FileParser::splitBySpaces(std::string &line, size_t &prev, size_t &end)
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
			m_frags.push_back(line.substr(prev, end - prev));
		prev = end + 1;
	}
	return ++end;
}

void	FileParser::getFile(std::string file)
{
	std::string				line;
	size_t					prev, end;

	prev = 0;
	end = 0;
	line = fileToStr(file);
	while (end < line.size())
		splitBySpaces(line, prev, end);
	if (prev != end)
		m_frags.push_back(line.substr(prev, end));
}

void	FileParser::setDefault(AConfig& obj) {
	std::deque<std::string>				frag;
	size_t								pos;

	while ((pos = findEndCharsetFromStrs(';')) != (size_t)-1) {
		m_frags[pos].erase(m_frags[pos].size() - 1);
		frag.insert(frag.end(), m_frags.begin(), m_frags.begin() + pos + 1);
		obj.addDefaultVal(frag);
		frag.clear();
		m_frags.erase(m_frags.begin(), m_frags.begin() + pos + 1);
	}
}

size_t	FileParser::findEndCharsetFromStrs(char end) {
	size_t	pos;

	for (pos = 0; pos < m_frags.size(); pos++) {
		if (m_frags[pos][m_frags[pos].size() - 1] == end)
			return pos;
	}
	return (size_t) -1;
}

std::string	FileParser::fileToStr(std::string file)
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