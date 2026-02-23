#ifndef KQUEUE_HPP
# define KQUEUE_HPP

# include <iostream>
# include <sys/event.h>
# include <stdexcept> 
# include <unistd.h>
# include <map>
# include <fcntl.h>

# include "../exception/Exception.hpp"

typedef enum e_event_type {
	LISTEN,
	ACCEPT,
	CONNECT,
	CGI,
	PID,
	SHUTDOWN
} t_event_type;

typedef struct s_udata {
	t_event_type	event_type;
	int				listen_fd;
	int				accept_fd;
} 	t_udata;

typedef std::map<int, t_udata> t_udata_map;

class Kqueue {
	private:
		static Kqueue	*mp_instance;
		int				m_kq;
		/* OCCF */
						Kqueue();
						~Kqueue();
		/* print */
		void			printUdata(int fd);
	public:
		/* get */
		static Kqueue	*getInstance();
		static int		getKqueue();
		/* valid */
		bool			isValidFd(int fd);
		/* add */
		void			addReadEvent(int add_r_fd, int listen_fd, t_event_type event_type);	
		void			addReadEvent(int add_r_fd, int listen_fd, int accept_fd, t_event_type event_type);	
		void			addWriteEvent(int add_w_fd, int listen_fd, t_event_type event_type);
		/* delete */
		void			deleteReadEvent(int delete_r_fd);
		void			deleteWriteEvent(int delete_w_fd);
		void			deleteReadWriteEvent(int delete_rw_fd);
		/* delete + add */
		void 			deleteaddReadEvent(int delete_r_fd, int add_r_fd, int listen_fd, int accept_fd, t_event_type event_type);
		void 			deleteaddWriteEvent(int delete_w_fd, int add_w_fd, int listen_fd, t_event_type event_type);
		void			deleteReadaddWriteEvent(int delete_r_fd, int add_w_fd, int listen_fd, t_event_type event_type);
		void			deleteWriteAddReadEvent(int delete_w_fd, int add_r_fd, int listen_fd, t_event_type event_type);
	};

#endif