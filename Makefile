NAME = webserv

CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
# CXXFLAGS = -std=c++98
#CPPFLAGS 
SRCS = ./backend/main.cpp \
		./backend/Server.cpp \
		./backend/ServerUtils.cpp \
		./backend/ServerManager.cpp \
		./backend/Controller.cpp \
		./backend/Model.cpp \
		./backend/View.cpp \
		./backend/Cgi.cpp \
		./backend/SessionManager.cpp \
		./backend/HttpUtils.cpp \
		./backend/HttpMsg.cpp \
		./backend/Kqueue.cpp \
		./exception/Exception.cpp \
		./config/Parser.cpp \
		./config/FileParser.cpp \
		./config/AConfig.cpp \
		./config/AppConfig.cpp \
		./config/MainConfig.cpp \
		./config/ServerConfig.cpp \
		./config/LocationConfig.cpp \
		./checker/Checker.cpp \
		./checker/SyntaxChecker.cpp

OBJS = $(SRCS:.cpp=.o)

#bonus
all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

%.o : %.cpp %.hpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean
	make all

.PHONY: all clean fclean re
