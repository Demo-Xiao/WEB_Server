#include "ThreadsPool.h"

ThreadsPool::ThreadsPool( int _queueCap, int _minNum, int _maxNum)
	: queueCap(_queueCap), minNum(_minNum), maxNum(_maxNum)
{

	shutDown = 0;
  
	queueFront = 0;
	queueRear = 0;   
	queueSize = 0;


	if (pthread_mutex_init(&mutexPool, NULL) != 0 || pthread_cond_init(&notFull, NULL) != 0 || pthread_cond_init(&notEmpty, NULL) != 0)
	{
		std::cout << "fail to init!\n";
	}

	workerThreadID = new pthread_t[_maxNum];
	tasks = new Task[_queueCap];
}

ThreadsPool::~ThreadsPool() 
{
	delete workerThreadID;
	delete tasks;
}

void ThreadsPool::threadInit()
{
	pthread_create(&managerThreadID, NULL, managerJob, this);
	for (int i = 0; i < minNum; i++) 
	{
		pthread_create(&workerThreadID[i], NULL, workerJob, this);
		pthread_mutex_lock(&mutexPool);
		liveNum++;
		pthread_mutex_unlock(&mutexPool);
	}
	std::cout << "the init num of threads is " << liveNum << std::endl;
}

void* ThreadsPool::managerJob(void* arg) 
{
	sleep(1);
	ThreadsPool* pool = (ThreadsPool*)arg;
	std::cout << "***********" << std::endl;
	std::cout << "the num of threads is :"<< pool->liveNum << std::endl;
	std::cout << "***********" << std::endl;

	while (!pool->shutDown) 
	{
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->queueSize;
		int liveNum = pool->liveNum;
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexPool);

		if ((queueSize > liveNum) && (liveNum < pool->maxNum)) 
		{
			int counter = 0;
			//记录这个构成增加了几个线程

			//动态增加线程
			pthread_mutex_lock(&pool->mutexPool);
			for (int i = 0; (i < pool->maxNum) && (pool->liveNum < pool->maxNum); i++) 
			{
				if (pool->workerThreadID[i] == 0) 
				{
					pthread_create(&pool->workerThreadID[i],NULL,workerJob,pool);
					counter++;
					pool->liveNum++;
					liveNum = pool->liveNum;
					busyNum = pool->busyNum;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}
		//动态销毁线程
		if (busyNum * 2 < liveNum && liveNum > pool->minNum) 
		{
			std::cout << "time to start delete threads!\n";
			pthread_mutex_lock(&pool->mutexPool);
			std::cout << "alter the exitNum!\n";
			pool->exitNum = 2;  // 此处2代表一次销毁两个
			
			for (int i = 0; i < 2; i++)
			{
				pthread_cond_signal(&pool->notEmpty);
				std::cout << "signal the not Empty!\n";
			}
			
			pthread_mutex_unlock(&pool->mutexPool);
		}

		
	}
}


void* ThreadsPool::workerJob(void* arg)
{
	ThreadsPool* pool = (ThreadsPool*)arg;
	while (1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		std::cout << "THE THREAD : [" << pthread_self() << "] has fitch the lock" << std::endl;
		std::cout << "*************" << std::endl;
		std::cout << "the num of threads is:" << pool->liveNum << std::endl;
		while (pool->queueSize == 0 && !pool->shutDown)//当任务队列的size == 0时，或者线程池标记为关闭的时候，阻塞等待
		{
			std::cout << " Waitting for the task!" << std::endl;
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
			if (pool->exitNum > 0)
			{
				std::cout << "to delete threads" << std::endl;
				std::cout << "the liveNum is : " << pool->liveNum << std::endl;
				pool->exitNum--;
				
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					std::cout << "the liveNum is : " << pool->liveNum << std::endl;
					pthread_mutex_unlock(&pool->mutexPool);
					pool->threadExit();
				}
				
			}
		}

		if (pool->shutDown)
		{
			//pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}
		std::cout << "begin to process the job" << std::endl;
		Task NowTask;
		NowTask.function = pool->tasks[pool->queueFront].function;
		NowTask.arg = pool->tasks[pool->queueFront].arg;

		pool->queueFront = (pool->queueFront + 1) % pool->queueCap;
		pool->queueSize--;
		pthread_cond_signal(&pool->notFull);
		pthread_mutex_unlock(&pool->mutexPool);

		//更改busyNum
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);

		NowTask.function(NowTask.arg);

		//任务执行完毕之后再次修改busyNum;
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}
}

void ThreadsPool::addTask(Task* task)
{
	pthread_mutex_lock(&mutexPool);

	while ((queueSize == queueCap) && (!shutDown))
	{
		pthread_cond_wait(&notFull, &mutexPool);
	}

	if (shutDown) {
		pthread_mutex_unlock(&mutexPool);
		return;
	}

	tasks[queueRear] = *task;
	queueRear = (queueRear + 1) % queueCap;
	queueSize++;
	std::cout << "the size of queueSize is:" << queueSize << std::endl;
	pthread_cond_signal(&notEmpty);
	pthread_mutex_unlock(&mutexPool);
	
}

void ThreadsPool::threadExit() 
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < maxNum; i++) {
		if (tid == workerThreadID[i]) {
			workerThreadID[i] = 0;
			pthread_exit(NULL);
		}
	}
}

