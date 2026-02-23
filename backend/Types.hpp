#ifndef TYPES_HPP
# define TYPES_HPP

# include <iostream>
# include <vector>
# include <map>

# include "HttpMsg.hpp"

typedef std::pair<std::string, std::string> t_pair;
typedef std::vector<std::vector<std::string> > t_vec_vec;

typedef enum e_sock_status {
	SOCKET_OPEN,
	SOCKET_CLOSE,
	SOCKET_NEXT
}	t_sock_status;

typedef struct s_accept {
	int					header_size, body_size;
	std::string			proxy_pass, sessionid;
	std::pair<int, int>	cgi_fd;
	HttpMsg				request, response;
}			t_accept;

typedef std::map<int, t_accept> t_accept_map;

typedef struct s_connection {
	std::map<int, int>	accept_key; // <accept_fd, connect_fd>
	std::map<int, int>	connect_key; // <coonect_fd, accept_fd>
}						t_connection;

typedef struct s_listen {
	t_connection	connection; // accept connect mapping
	t_accept_map	accept_map; // <accept_fd, t_accept>
}					t_listen;

typedef std::map<int, t_listen> t_listen_map; // <listen_fd, t_listen>

#endif