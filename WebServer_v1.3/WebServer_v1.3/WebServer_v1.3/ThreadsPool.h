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

	void start(int numThreads); // 创建线程
	void addTask(Task& task); // 往线程池中添加任务
	void runInThread(); // 每个线程需要进行的工作，也就是工作线程的回调
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