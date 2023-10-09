#include "LogFile.h"

LogFile::LogFile(int maxflush, const std::string& fileName)
	:flushEveryN_(maxflush), count_(0), fileName_(fileName), file_(new AppendFile(fileName))
{
	pthread_mutex_init(&mutexFile_, NULL); // ³õÊ¼»¯»¥³âËø
}

LogFile::~LogFile()
{
	pthread_mutex_destroy(&mutexFile_);
}

void LogFile::append(const char* logInfo, int len)
{
	pthread_mutex_lock(&mutexFile_);
	file_->append(logInfo, len);
	count_++;
	if (count_ >= flushEveryN_) 
	{
		count_ = 0;
		file_->flush();
	}
	pthread_mutex_unlock(&mutexFile_);
}

void LogFile::flush()
{
	pthread_mutex_lock(&mutexFile_);
	file_->flush();
	pthread_mutex_unlock(&mutexFile_);
}
