#pragma once
#include <sys/epoll.h> 
#include <fcntl.h>  
#include <unistd.h> 
#include <assert.h> 
#include <vector>
#include <stdio.h>
class EpollObj 
{
public:
	int epollFd;
	std::vector<epoll_event> possibleEvs;
public:
	EpollObj(int maxEvent);
	~EpollObj();

	int EpollAdd(int fd, uint32_t events);

	bool EpollDelete(int fd);

	int EpollWait();//返回发生变换的事件数

	int GetEventFd(int i); // 返回发生事件的标识符

	uint32_t GetEvents(size_t i) const;
};