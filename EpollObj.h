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

	int EpollWait();//���ط����任���¼���

	int GetEventFd(int i); // ���ط����¼��ı�ʶ��

	uint32_t GetEvents(size_t i) const;
};