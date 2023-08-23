#include "ThreadsPool.h"

void* ThreadsPool::callBackfun(void* arg) 
{
	ThreadsPool* pool = static_cast<ThreadsPool*>(arg);
	pool->runInThread();
	//delete pool;  ����ط��о����ܻ����
	return nullptr;
}

void ThreadsPool::stop()
{
	printf("stop()!!!");
	pthread_mutex_lock(&mutexPool_);
	running_ = false;
	pthread_cond_broadcast(&notEmpty_);
	for (int i = 0; i < numThreads_; i++) 
	{
		pthread_join(ThreadId_[i], NULL);
	}

	while (!taskQueue_.empty()) 
	{
		taskQueue_.pop();
	}
}

ThreadsPool::ThreadsPool(int maxQueueSize)
	:maxQueueSize_(maxQueueSize), numThreads_(0), running_(false)
{
	pthread_mutex_init(&mutexPool_, NULL);
	pthread_cond_init(&notEmpty_, NULL);
	pthread_cond_init(&notFull_, NULL);
	printf("ThreadsPool has been created!\n");
}

ThreadsPool::~ThreadsPool()
{
	if (running_) 
	{
		stop();
	}
	pthread_mutex_destroy(&mutexPool_);
	pthread_cond_destroy(&notEmpty_);
	pthread_cond_destroy(&notFull_);
	printf("ThreadsPool has been deleted!\n");
}

void ThreadsPool::start(int numThreads)
{
	printf("begin create threads!\n");
	numThreads_ = numThreads;
	running_ = true;
	ThreadId_.resize(numThreads_);
	for (int i = 0; i < numThreads_; i++) 
	{
		//�����̡߳�
		// ��Ҫ֪������߳�Ҫ��ʲô�£�Ҳ���ǻص�
		pthread_create(&ThreadId_[i], NULL, &callBackfun, this);
	}
	printf("threads has been created!\n");
}



void ThreadsPool::addTask(Task& task)
{
	pthread_mutex_lock(&mutexPool_);
	while (taskQueue_.size() >= maxQueueSize_) 
	{
		pthread_cond_wait(&notFull_, &mutexPool_);
	}
	taskQueue_.push(std::move(task));
	pthread_cond_signal(&notEmpty_);
	pthread_mutex_unlock(&mutexPool_);
}

void ThreadsPool::runInThread()
{
	//���ϵ�ȡ����
	while (running_) 
	{
		pthread_mutex_lock(&mutexPool_);
		while (taskQueue_.empty() && running_)
		{
			// �������Ϊ��
			pthread_cond_wait(&notEmpty_, &mutexPool_);
		}

		Task task;
		if (!taskQueue_.empty()) 
		{
			task = taskQueue_.front();
			taskQueue_.pop();

			pthread_cond_signal(&notFull_);
		}
		pthread_mutex_unlock(&mutexPool_);

		if (task) 
		{
			task();
		}
	}
}
