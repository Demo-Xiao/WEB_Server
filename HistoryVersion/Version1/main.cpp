#include"serverEpoll.h"

#include<iostream>


int main() 
{
	//�л�����Ŀ¼
//��ȡ��ǰĿ¼�Ĺ���·��

	char pwd_path[256] = "";
	char* path = getenv("PWD");
	strcpy(pwd_path, path);
	strcat(pwd_path, "/web-http");
	chdir(pwd_path);

	std::cout << getenv("PWD") << std::endl;
	std::cout << " web_server satrt! " << std::endl;
	

	ThreadsPool* pool = new ThreadsPool(10, 100);
	pool->threadInit();

	epollRun(pool);

}