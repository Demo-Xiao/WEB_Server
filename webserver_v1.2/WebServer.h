#pragma once
#include "EpollObj.h"
#include "HttpConn.h"
#include "ThreadsPool.h"
#include <iostream>
#include <functional>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
class WebServer 
{
public:
	EpollObj* myEpoll;
	const int Port;
	int listenFd;
	uint32_t listenEvent;
	//std::vector<HttpConn*>* clients;  // ÐèÒª´æ´¢fd
	std::unordered_map<int, HttpConn*> clients;
	ThreadsPool* Pool;

public:
	int SocketBindListen();
	int	SetSocketNonBlocking(int fd);
	void Start();
	void DealListen();
	void DealRead(HttpConn* client);
	void AddClient(int fd, sockaddr_in addr);
	void DeleteClient(HttpConn* client);
public:
	WebServer(int port);
	~WebServer();
};