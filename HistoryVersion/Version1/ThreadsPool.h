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
	void (*function)(void* arg);//�������Ҫִ�еú���
	void* arg;//��������ThreadPool
};

class ThreadsPool 
{
public:

	Task* tasks;

	int shutDown;  //��¼�̳߳��Ƿ񱻹ر�

	int threadsSum;   //�߳�����
	int queueCap;    //������е�����
	int queueFront;  //������е�ͷ
	int queueRear;   //������е�β
	int queueSize;   //��ǰ������еĳ���

	pthread_t *workerThreadID; // �������߳�
	pthread_t managerThreadID;// �������߳�

	pthread_mutex_t mutexPool; // ���̳߳ز���ʱ����
	pthread_cond_t notFull;//  �����������ȴ��������û�����ͽ������
	pthread_cond_t notEmpty;//   �����������ǿվͽ������

public:
	
	ThreadsPool() = delete;
	ThreadsPool(int _threadSum, int _queueCap);

	~ThreadsPool();

	void threadInit();
	void threadExit();
	void addTask(Task* task);


private:
	

	//static void* managerJob(void* arg);  // �����̣߳���̬�����̵߳�����

	static void* workerJob(void* arg);


};
