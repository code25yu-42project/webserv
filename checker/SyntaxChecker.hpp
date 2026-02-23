#ifndef SYNTAXCHECKER_HPP
# define SYNTAXCHECKER_HPP
# include <iostream>
# include <string>
# include <deque>
# include "Checker.hpp"
# include "../exception/Exception.hpp"

class SyntaxChecker {
    public:
        static void checkSyntax(const std::deque<std::string> &deq);
        static unsigned int	checkValidNum(std::string num_str, int max);
        static unsigned int	makeIP(std::string ip_str);
};

#endif