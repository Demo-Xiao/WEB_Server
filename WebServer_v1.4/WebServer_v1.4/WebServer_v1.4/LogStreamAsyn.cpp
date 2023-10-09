#include "LogStreamAsyn.h"

LogStreamAsyn::LogStreamAsyn(const std::string& fileName)
	:fileName_(fileName)
{
	pthread_mutex_init(&mutexAsynStream_,NULL);
	pthread_cond_init(&bufferNotEmpty_, NULL);
}

LogStreamAsyn::~LogStreamAsyn()
{
	if (running_) 
	{
		stop();
	}
	pthread_mutex_destroy(&mutexAsynStream_);
	pthread_cond_destroy(&bufferNotEmpty_);
	printf("LogStreamAsyn has been deleted!\n");

}

LogStreamAsyn& LogStreamAsyn::operator<<(const std::string& str)
{
	// TODO: insert return statement here
	std::string combinedString = str + "\n";
	append(combinedString.c_str(), combinedString.size());
	return *this;
}

void LogStreamAsyn::start()
{
	running_ = true;
	pthread_create(&threadId_, NULL, &callBackfun, this);
}

void* LogStreamAsyn::callBackfun(void* arg)
{
	LogStreamAsyn* AsynStream = static_cast<LogStreamAsyn*>(arg);
	AsynStream->threadFunc();

	return nullptr;
}

void LogStreamAsyn::threadFunc()
{

	LogFile logFile(1, fileName_);
	while (running_)
	{
		//printf("%s\n", "the thread is waiting for the mutexAs");
		pthread_mutex_lock(&mutexAsynStream_);
		//printf("%s\n", "the thread is get the mutexAs");

		while (buffers_.empty()) 
		{
			//printf("%s\n", "the thread is waiting for cond notEmpty");
			pthread_cond_wait(&bufferNotEmpty_, &mutexAsynStream_);
		}
		FixedBuffer& bufferToWrite = buffers_.front();
		logFile.append(bufferToWrite.data(), bufferToWrite.length());
		buffers_.pop();
		pthread_mutex_unlock(&mutexAsynStream_);
		//printf("%s\n", "the thread is loss the mutexAs");

	}
}

void LogStreamAsyn::testFunc()
{
	LogFile logFile(1024, fileName_);

	if (!buffers_.empty()) 
	{
		FixedBuffer& bufferToWrite = buffers_.front();
		logFile.append(bufferToWrite.data(), bufferToWrite.length());
		buffers_.pop();
	}
	else 
	{
		std::cout << "there is no data!!" << std::endl;
	}
	
}

void LogStreamAsyn::append(const char* logline, int len)
{
	//printf("%s\n", "begin append, waiting the mutexLock!");
	pthread_mutex_lock(&mutexAsynStream_);
	//printf("%s\n", "the append has feached the mutexLock");
	if (currentBuffer_.avail()  >= len) 
	{

		currentBuffer_.append(logline, len);
		//printf("%s", "the length of buffers_ is: ");
		//printf("%d\n", currentBuffer_.length());
	}
	else 
	{
		//buffers_.push(std::move(currentBuffer_));
		//printf("%s\n", "fulled");
		buffers_.push(currentBuffer_);
		pthread_cond_signal(&bufferNotEmpty_);
		//printf("%s\n", "Infor the buffer not Empty");
		/*
		if (nextBuffer_) 
		{
			currentBuffer_ = std::move(nextBuffer_);
		}
		else 
		{
			currentBuffer_.reset();
		}
		*/
		//printf("%s", "0the length of buffers_.front() is: ");
		//printf("%d\n", buffers_.front().length());
		currentBuffer_.reset();
		currentBuffer_.append(logline, len);
		//printf("%s", "1the length of buffers_.front() is: ");
		//printf("%d\n", buffers_.front().length());
	}



	pthread_mutex_unlock(&mutexAsynStream_);
	//printf("%s\n", "the append has lose the mutexLock");
}

void LogStreamAsyn::stop()
{
	pthread_mutex_lock(&mutexAsynStream_);
	running_ = false;
	pthread_cond_broadcast(&bufferNotEmpty_);
	pthread_join(threadId_, NULL);
	pthread_mutex_unlock(&mutexAsynStream_);
}
