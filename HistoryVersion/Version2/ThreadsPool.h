#pragma once

#include<iostream>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream> 

struct Task
{
	void (*function)(void* arg);//这个任务要执行得函数
	void* arg;//函数参数ThreadPool
};

class ThreadsPool 
{
public:

	Task* tasks;

	int shutDown;  //记录线程池是否被关闭

	int threadsSum;   //线程总数
	int queueCap;    //任务队列的容量
	int queueFront;  //任务队列的头
	int queueRear;   //任务队列的尾
	int queueSize;   //当前任务队列的长度


	pthread_t *workerThreadID; // 工作者线程
	pthread_t managerThreadID;// 管理者线程

	//实现动态增减的线程池
	int minNum;
	int maxNum;
	int liveNum;     //当前存在的线程数  
	int busyNum;     //当前工作中的线程数
	int exitNum;     //经过动态杀死的线程数

	pthread_mutex_t mutexPool; // 对线程池操作时加锁
	pthread_cond_t notFull;//  条件变量，等待任务队列没有满就解除阻塞
	pthread_cond_t notEmpty;//   条件变量，非空就解除阻塞

	pthread_mutex_t mutexBusy; // 对线程池中的busyNum操作加锁

public:
	
	ThreadsPool() = delete;
	ThreadsPool(int _queueCap, int _minNum, int _maxNum);

	~ThreadsPool();

	void threadInit();
	void threadExit();
	void addTask(Task* task);


private:
	

	static void* managerJob(void* arg);  // 管理线程，动态管理线程的数量

	static void* workerJob(void* arg);


};
