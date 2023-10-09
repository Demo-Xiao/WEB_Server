#pragma once
#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>    
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <string.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "sqlQuery.h"
struct HttpRequest 
{
	std::string METHOD;
	std::string URI;
	std::string VERSION;
	//std::pair<std::string, std::string> requestBodys;
	std::map<std::string, std::string> requestBodys;
};

class HttpConn 
{
public:
	int fd;  //对应客户端的连接套接字
	sockaddr_in addr;
	HttpRequest httpRequest;
	sqlQuery& sql;
	
public:
	HttpConn(int fd1, sockaddr_in addr1);

public:
	int ReadRequest();

	void ParseRequest(char * requestBuffer);

	void ProcessRequest();

	void SendHeader(int statusCode, const char *descr, const char *type);

	int SendFile(const char* filename);

	int GetFd();

	void CloseFd();

	const char* GetFileType(const char* name);
private:
	std::vector<std::string> split(const std::string& str, char delimiter);
};