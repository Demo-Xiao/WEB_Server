#include "ThreadsPool.h"

ThreadsPool::ThreadsPool(int _threadSum, int _queueCap)
	:threadsSum(_threadSum), queueCap(_queueCap)
{

	shutDown = 0;
  
	queueFront = 0;
	queueRear = 0;   
	queueSize = 0;


	if (pthread_mutex_init(&mutexPool, NULL) != 0 || pthread_cond_init(&notFull, NULL) != 0 || pthread_cond_init(&notEmpty, NULL) != 0)
	{
		std::cout << "fail to init!\n";
	}

	workerThreadID = new pthread_t[_threadSum];
	tasks = new Task[_queueCap];
}

ThreadsPool::~ThreadsPool() 
{
	delete workerThreadID;
	delete tasks;
}

void ThreadsPool::threadInit()
{
	//pthread_create(&managerThreadID, NULL, managerJob, this);
	for (int i = 0; i < threadsSum; i++) 
	{
		pthread_create(&workerThreadID[i], NULL, workerJob, this);
	}
}

void* ThreadsPool::workerJob(void* arg)
{
	ThreadsPool* pool = (ThreadsPool*)arg;
	while (1)
	{
		std::cout << "begin workerJob!\n";
		pthread_mutex_lock(&pool->mutexPool);

		std::cout << "THE thread :" << pthread_self() << "has fitch the lock" << std::endl;
		while (pool->queueSize == 0 && !pool->shutDown)
		{
			std::cout << " I am waitting for the task!" << std::endl;
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
		}

		if (pool->shutDown)
		{
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

		NowTask.function(NowTask.arg);
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
	for (int i = 0; i < threadsSum; i++) {
		if (tid == workerThreadID[i]) {
			workerThreadID[i] = 0;
			pthread_exit(NULL);
		}
	}
}

