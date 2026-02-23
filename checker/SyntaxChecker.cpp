#include "SyntaxChecker.hpp"

void    SyntaxChecker::checkSyntax(const std::deque<std::string> &deq)
{
	// std::cerr << deq[0] << " " << deq[1] << std::endl;
    if (deq.size() == 0)
		Exception::handleInvalidFile();
    else if (deq[0] != "internal" && deq.size() == 1)
		Exception::handleInvalidFile();
    else if (deq[0] == "error_page" && deq.size() <= 2)
		Exception::handleInvalidFile();
    else if (deq[0] == "server_names") {
        for (size_t i = 1; i < deq.size(); i++)
		{
			if (deq[i].find('*') != std::string::npos)
			{
				if (deq[i][0] != '*' && deq[i][deq[i].size() - 1] != '*')
					Exception::handleInvalidFile();
				else if ((deq[i][0] == '*' && deq[i][1] != '.') || deq.size() == 1)
					Exception::handleInvalidFile();
				else if (deq[i][deq[i].size() - 1] == '*' && deq[i][deq[i].size() - 2] != '.')
					Exception::handleInvalidFile();
			}
		}
    }
    else if (deq[0] == "limit_except") {
        for (size_t i = 1; i < deq.size(); i++)
        {
            if (deq[i] != "DELETE" && deq[i] != "POST" && deq[i] != "GET" && deq[i] != "HEAD" && deq[i] != "PUT" && deq[i] != "PATCH")
				Exception::handleInvalidFile();
        }
    }
    else if (deq[0] == "return"){
        if (deq.size() != 3)
			Exception::handleInvalidFile();
        if (deq[1].size() != 3 || deq[1] < "100" || deq[1] > "599")
			Exception::handleInvalidFile();
    }
	else if (deq[0] == "alias") {
		if (deq.size() != 2 || Checker::isExecutableDirectory(deq[1]) == false)
			Exception::handleInvalidFile();
	}
    else if (deq.size() == 1)
		Exception::handleInvalidFile();
}

unsigned int	SyntaxChecker::checkValidNum(std::string num_str, int max)
{
	char	*ptr;
	double	d;

	d = strtod(num_str.c_str(), &ptr);
	if (num_str == "" || (*ptr) || d > max || d < 0 || static_cast<double>(static_cast<unsigned int>(d)) != d) {
		std::cerr << "strfy error: " << num_str << std::endl;
		Exception::handleInvalidFile();
	}
	return static_cast<unsigned int>(d);
}

unsigned int	SyntaxChecker::makeIP(std::string ip_str)
{
	size_t			idx, pos, npos;
	unsigned int	ip, num;

	ip = 0; idx = 0; pos = 0;
	if (ip_str == "*")
		return ip;
	if (ip_str == "localhost")
		ip_str = "127.0.0.1";
	while (idx < 4)
	{
		if ((npos = ip_str.find('.', pos)) == (size_t)-1 && idx != 3) {
			std::cerr << "error IP" << std::endl;
			Exception::handleInvalidFile();
		}
		num = checkValidNum(ip_str.substr(pos, npos - pos), 255);
		ip += num << (8 * idx);
		pos = npos + 1;
		idx++;
	}
	return ip;
}