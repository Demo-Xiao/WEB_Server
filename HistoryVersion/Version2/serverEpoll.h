#pragma once
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iconv.h>
#include <ctype.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include "ThreadsPool.h"
#define PORT 8888
#define Possible_evs_Length 1024

struct ForParameter 
{
	int fd;
	epoll_event ev;
};


struct HttpRequest //存放客户段的请求信息
{
	std::string METHOD;
	std::string CONTENT;
	std::string PROTOCOL;
};

int SocketBindListen();

int epollRun(ThreadsPool* pool);

HttpRequest parseRequest(char* buf);

void processRequest(HttpRequest Request, epoll_event* ev);

void sendHeader(epoll_event* ev, int statusCode, const char* descr, const char* type);

int epoll_add(int fd, int epfd, struct epoll_event& ev);

int Connect_create(int epfd, int lfd, epoll_event events);  //简历服务器与客户端之间的连接

const char* getFileType(const char* name);  //根据文件类型发送相应文件信息

void ReadClientRequest(int epfd, epoll_event* ev);

int sendFile(const char* filename, int cfd);

int sendDir(const char* dirName, int cfd);// 

void processWork(void* arg);