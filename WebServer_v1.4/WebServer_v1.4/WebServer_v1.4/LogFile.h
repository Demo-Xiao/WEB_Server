#pragma once
#include <memory>
#include "FileUtil.h"
class LogFile 
{
public:
	LogFile(int maxflush, const std::string &fileName);
	~LogFile();

	void append(const char* logInfo, int len);
	void flush();

//private:

	const int flushEveryN_;

	int count_;
	std::string fileName_;

	pthread_mutex_t mutexFile_;
	std::unique_ptr<AppendFile> file_;
	//AppendFile file_;
};