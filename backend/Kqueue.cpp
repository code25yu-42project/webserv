#include "Kqueue.hpp"

t_udata_map udata_map;
Kqueue* Kqueue::mp_instance = NULL;

/* ************************************************************************** */
/* ------------------------------	 OCCF	------------------------------- */
/* ************************************************************************** */

/* private */
Kqueue::Kqueue() : m_kq(-1) {
	// std::cout << "[Class Kqueue] Kqueue initialized." << std::endl; 
}

Kqueue::~Kqueue() {
	if (m_kq != -1) {
		close(m_kq);
		// std::cout << "[Class Kqueue] Successfully closed Kqueue fd: " << m_kq << std::endl;
	} else {
		// std::cout << "[Class Kqueue] Kqueue fd already closed or not initialized." << std::endl;
	}
	if (mp_instance) {
		delete mp_instance;
		// std::cout << "[Class Kqueue] Deleted singleton instance of Kqueue." << std::endl;
	} else {
		// std::cout << "[Class Kqueue] Singleton instance was already null, no deletion needed." << std::endl;
	}
	// std::cout << "[Class Kqueue] Kqueue instance destroyed." << std::endl;
}

/* ************************************************************************** */
/* --------------------------------   GET    -------------------------------- */
/* ************************************************************************** */

/* public */
Kqueue* Kqueue::getInstance() {
	if (!mp_instance) {
		mp_instance = new Kqueue();
		mp_instance->m_kq = kqueue();
		if (mp_instance->m_kq == -1)
			Exception::handleSystemError(errno);
	}
	return (mp_instance);
}

int Kqueue::getKqueue() {
	if (mp_instance && mp_instance->m_kq != -1)
		return (mp_instance->m_kq);
	return (-1);
}

/* ************************************************************************** */
/* --------------------------------   VALID    ------------------------------ */
/* ************************************************************************** */

bool	Kqueue::isValidFd(int fd) {
	return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

/* ************************************************************************** */
/* ---------------------------------   ADD    ------------------------------- */
/* ************************************************************************** */

/* public */
void	Kqueue::addReadEvent(int read_fd, int listen_fd, t_event_type event_type) {
	struct kevent	event;
	t_udata			udata;

	if (isValidFd(read_fd) == false)
		return ;
	udata.event_type = event_type;	
	udata.listen_fd = listen_fd;
	udata.accept_fd = -1;
	udata_map[read_fd] = udata;
	printUdata(read_fd);
	EV_SET(&event, read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &udata_map[read_fd]);
	if (kevent(mp_instance->m_kq, &event, 1, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
}

void Kqueue::addReadEvent(int read_fd, int listen_fd, int accept_fd, t_event_type event_type) {
	struct kevent	event;
	t_udata			udata;

	if (isValidFd(read_fd) == false)
		return ;
	udata.event_type = event_type;	
	udata.listen_fd = listen_fd;
	udata.accept_fd = accept_fd;
	udata_map[read_fd] = udata;
	printUdata(read_fd);
	EV_SET(&event, read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &udata_map[read_fd]);
	if (kevent(mp_instance->m_kq, &event, 1, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
	// std::cout << "Adding READ event for fd: " << read_fd << std::endl;
	
}

void	Kqueue::addWriteEvent(int write_fd, int listen_fd, t_event_type event_type) {
	struct kevent	event;
	t_udata			udata;

	if (isValidFd(write_fd) == false)
		return ;
	// 기존 READ 이벤트 제거 후 WRITE 이벤트 추가
	// std::cout << "Adding WRITE event for fd: " << write_fd << std::endl;
	udata.event_type = event_type;	
	udata.listen_fd = listen_fd;
	udata.accept_fd = -1;
	udata_map[write_fd] = udata;
	printUdata(write_fd);
	EV_SET(&event, write_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &udata_map[write_fd]);
	if (kevent(mp_instance->m_kq, &event, 1, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
}

/* ************************************************************************** */
/* ---------------------------------  DELETE  ------------------------------- */
/* ************************************************************************** */

/* public */
void Kqueue::deleteReadEvent(int fd) {
	struct kevent event;

	if (isValidFd(fd) == false)
		return ;
	if (isValidFd(fd) == false)
		return ;
	EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(mp_instance->m_kq, &event, 1, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
	// std::cout << "[Event] Successfully removed READ event for socket fd: " << fd << std::endl;
}

void Kqueue::deleteWriteEvent(int fd) {
	struct kevent event;

	if (isValidFd(fd) == false)
		return ;
	EV_SET(&event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	if (kevent(mp_instance->m_kq, &event, 1, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
	// std::cout << "[Event] Successfully removed WRITE event for socket fd: " << fd << std::endl;
}

void Kqueue::deleteReadWriteEvent(int fd) {

	if (isValidFd(fd) == false)
		return ;
	try{
		deleteWriteEvent(fd);
	} catch (std::exception &e) {
		// std::cout << "[Event] No kqueue Write events for socket: " << fd << std::endl;
	}
	try {
		deleteReadEvent(fd);
	} catch (std::exception &e) {
		// std::cout << "[Event] No kqueue Read events for socket: " << fd << std::endl;
	}
}

/* ************************************************************************** */
/* ---------------------------  DELETE + ADD   ------------------------------ */
/* ************************************************************************** */

/* public */
void	Kqueue::deleteReadaddWriteEvent(int read_fd, int write_fd, int listen_fd, t_event_type event_type) {
	struct kevent	events[2];
	t_udata			udata;

	if (isValidFd(read_fd) == false || isValidFd(write_fd) == false)
		return ;
	
	// 기존 READ 이벤트 제거 후 WRITE 이벤트 추가
	// std::cout << "Deleting READ event for fd: " << read_fd << " and adding WRITE event for fd: " << write_fd << std::endl;

	udata.event_type = event_type;	
	udata.listen_fd = listen_fd;
	udata.accept_fd = -1;
	udata_map[write_fd] = udata;
	printUdata(write_fd);
	
	EV_SET(&events[0], read_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	EV_SET(&events[1], write_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &udata_map[write_fd]);
	if (kevent(mp_instance->m_kq, events, 2, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
}

void	Kqueue::deleteWriteAddReadEvent(int write_fd, int read_fd, int listen_fd, t_event_type event_type) {
	struct kevent	events[2];
	t_udata			udata;

	if (isValidFd(read_fd) == false || isValidFd(write_fd) == false)
		return ;

	// 기존 WRITE 이벤트 제거 후 READ 이벤트 추가
	// std::cout << "Deleting WRITE event for fd: " << write_fd << " and adding READ event for fd: " << read_fd << std::endl;

	udata.event_type = event_type;	
	udata.listen_fd = listen_fd;
	udata.accept_fd = -1;
	udata_map[read_fd] = udata;
	printUdata(read_fd);
	
	EV_SET(&events[0], write_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	EV_SET(&events[1], read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &udata_map[read_fd]);
	if (kevent(mp_instance->m_kq, events, 2, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
}

void	Kqueue::deleteaddWriteEvent(int delete_write_fd, int add_write_fd, int listen_fd, t_event_type event_type) {
	struct kevent	events[2];
	t_udata			udata;

	if (isValidFd(delete_write_fd) == false || isValidFd(add_write_fd) == false)
		return ;
	// std::cout << "Deleting WRITE event for fd: " << delete_write_fd << " and adding WRITE event for fd: " << add_write_fd << std::endl;

	// 기존 WRITE 이벤트 제거 후 같은 WRITE 이벤트 다시 추가
	udata.event_type = event_type;
	udata.listen_fd = listen_fd;
	udata.accept_fd = -1;
	udata_map[add_write_fd] = udata;
	printUdata(add_write_fd);
	EV_SET(&events[0], delete_write_fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	EV_SET(&events[1], add_write_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &udata_map[add_write_fd]);
	if (kevent(mp_instance->m_kq, events, 2, NULL, 0, NULL) == -1) {
		// perror("failed");
		// Exception::handleSystemError(errno);
		return ;
	}
	// std::cout << "Deleting WRITE event for fd: " << delete_write_fd << " and adding WRITE event for fd: " << add_write_fd << std::endl;
}

void	Kqueue::deleteaddReadEvent(int delete_read_fd, int add_read_fd, int listen_fd, int accept_fd, t_event_type event_type) {
	struct kevent	events[2];
	t_udata			udata;

	if (isValidFd(delete_read_fd) == false || isValidFd(add_read_fd) == false)
		return ;
	// 기존 WRITE 이벤트 제거 후 같은 WRITE 이벤트 다시 추가
	udata.event_type = event_type;
	udata.listen_fd = listen_fd;
	udata.accept_fd = accept_fd;
	udata_map[add_read_fd] = udata;
	printUdata(add_read_fd);
	EV_SET(&events[0], delete_read_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	EV_SET(&events[1], add_read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &udata_map[add_read_fd]);
	if (kevent(mp_instance->m_kq, events, 2, NULL, 0, NULL) == -1)
		// Exception::handleSystemError(errno);
		return ;
	// std::cout << "Deleting REAED  event for fd: " << delete_read_fd << " and adding READ event for fd: " << add_read_fd << std::endl;
}

/* ************************************************************************** */
/* -------------------------------- PRINT   --------------------------------- */
/* ************************************************************************** */

/* private */
void	Kqueue::printUdata(int fd) {
	std::cout << "[Kqueue Udata] ADDR: " << &udata_map
	<< "| EVENT_TYPE: " << udata_map[fd].event_type 
	<< "| Listen FD : " << udata_map[fd].listen_fd 
	<< "| Accept FD : " << udata_map[fd].accept_fd << std::endl;	
}

// void	addProcEvent(pid_t pid) {
// 	struct kevent event;
	
// 	EV_SET(&event, pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, NULL);
// 	if (kevent(m_kq, &event, 1, NULL, 0, NULL) == -1) {
		// Exception::handleSystemError(errno);
		// return ;
// 	}
	// std::cout << "Add PROC event for fd: " << pid << std::endl;
// }
