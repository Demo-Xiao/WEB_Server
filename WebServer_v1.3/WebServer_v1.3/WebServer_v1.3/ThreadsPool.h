#pragma once
#include"noncopyable.h"
#include<queue>
#include<vector>
#include <pthread.h>
#include <functional>

#include<iostream>
class ThreadsPool : noncopyable
{
public:
	typedef std::function<void()> Task;
	explicit ThreadsPool(int maxQueueSize);
	~ThreadsPool();

	void start(int numThreads); // �����߳�
	void addTask(Task& task); // ���̳߳����������
	void runInThread(); // ÿ���߳���Ҫ���еĹ�����Ҳ���ǹ����̵߳Ļص�
	static void* callBackfun(void*);
	void stop();

	pthread_mutex_t mutexPool_; //
	pthread_cond_t notFull_;//  
	pthread_cond_t notEmpty_;//



	bool running_; 
	std::queue<Task> taskQueue_;
	int maxQueueSize_;
	std::vector<pthread_t> ThreadId_;
	int numThreads_;
};