#include"serverEpoll.h"

#include<iostream>
void taskfunc(void* arg) {
	int i = *(int*)arg;
	pthread_t tid = pthread_self();
	printf("Number is %d  and thread ID is %ld\n", i, tid);
	sleep(1);
}

int main()
{

	ThreadsPool* pool = new ThreadsPool(100, 2,10);

	pool->threadInit();
	
	for (int i = 0; i < 100; i++)
	{

		int* num = new int[100];
		num[i] = i;
		printf("num = %d\n", num[i]);
		Task* task = new Task;
		task->arg = &(num[i]);
		task->function = taskfunc;
		pool->addTask(task);
	}
	sleep(100);
	//sleep(60);
	free(pool);
	return 0;

}






/*
int main() 
{
	//切换工作目录
//获取当前目录的工作路径

	char pwd_path[256] = "";
	char* path = getenv("PWD");
	strcpy(pwd_path, path);
	strcat(pwd_path, "/web-http");
	chdir(pwd_path);

	std::cout << getenv("PWD") << std::endl;
	std::cout << " web_server satrt! " << std::endl;
	

	ThreadsPool* pool = new ThreadsPool(100, 2, 10);
	pool->threadInit();

	epollRun(pool);

}
*/