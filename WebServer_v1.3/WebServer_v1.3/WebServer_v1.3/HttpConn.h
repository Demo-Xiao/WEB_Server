#pragma once
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>    
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include <iostream>
#include <string>

struct HttpRequest 
{
	std::string METHOD;
	std::string CONTENT;
	std::string PROTOCOL;
};

class HttpConn 
{
public:
	int fd;  //对应客户端的连接套接字
	sockaddr_in addr;
	HttpRequest httpRequest;

public:
	HttpConn(int fd1, sockaddr_in addr1);

public:
	int ReadRequest();

	void ParseRequest(char *buf);

	void ProcessRequest();

	void SendHeader(int statusCode, const char *descr, const char *type);

	int SendFile(const char* filename);

	int GetFd();

	void CloseFd();

	const char* GetFileType(const char* name);

};