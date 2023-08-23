#include "EpollObj.h"

EpollObj::EpollObj(int maxEvent)
	:epollFd(epoll_create(1)), possibleEvs(maxEvent)
{
	assert(epollFd >= 0 && possibleEvs.size() > 0);
}


EpollObj::~EpollObj() 
{
	
}

int EpollObj::EpollWait()
{
	int num = epoll_wait(epollFd, &possibleEvs[0], possibleEvs.size(), -1);
	// 这里没有用 static_cast<int>
	return num;
}

int EpollObj::GetEventFd(int i)
{
	assert(i < possibleEvs.size() && i >= 0);
	return possibleEvs[i].data.fd;
}

uint32_t EpollObj::GetEvents(size_t i) const
{
	assert(i < possibleEvs.size() && i >= 0);
	return possibleEvs[i].events;
}

int EpollObj::EpollAdd(int fd, uint32_t events)
{
	epoll_event ev = {0};
	ev.data.fd = fd;
	//ev->events = EPOLLIN | EPOLLET;  //设置ET模式
	ev.events = events;
	int ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
	if (ret < 0)
	{
		perror("epoll_ctl");
		return -1;
	}
	return 1;
}

bool EpollObj::EpollDelete(int fd)
{
	if (fd < 0) return false;
	return epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) == 0;
}
