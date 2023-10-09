#pragma once
#include<iostream>
#include<string>
#include<cstring>
#include <pthread.h>
#include <queue>
#include "LogFile.h" 

class FixedBuffer
{
public:
	FixedBuffer() : cur_(data_) {}
	FixedBuffer(const FixedBuffer& other) 
	{
		memcpy(data_, other.data_, sizeof(char) * (end() - data_));
		cur_ = data_ + (other.cur_ - other.data_);
	}
	~FixedBuffer() {}

	void append(const char* buf, size_t len) {
		if (avail() > static_cast<int>(len)) {
			memcpy(cur_, buf, len);
			cur_ += len;
		//	*(cur_ + 1) = '\n';
		}

	}

	const char* data() const { return data_; }
	int length() const { return static_cast<int>(cur_ - data_); }

	char* current() { return cur_; }
	int avail() const { return static_cast<int>(end() - cur_); }
	void add(size_t len) { cur_ += len; }

	void reset() { cur_ = data_; }
	void bzero() { memset(data_, 0, sizeof data_); }

	//private:
	const char* end() const { return data_ + sizeof data_; }

	char data_[40];
	char* cur_;
};


class LogStreamAsyn 
{
public:
	LogStreamAsyn(const std::string& fileName);
	~LogStreamAsyn();

	LogStreamAsyn& operator<< (const std::string& v);
	void start();

public:
	std::string fileName_;
	static void* callBackfun(void* arg);
	void threadFunc();
	void testFunc();
	void append(const char* logline, int len);
	bool running_;
	void stop();
	


	std::queue<FixedBuffer> buffers_;
	FixedBuffer currentBuffer_;  // 固定大小的buffer
	FixedBuffer nextBuffer_;  // 固定大小的buffer
	pthread_mutex_t mutexAsynStream_; // 互斥锁
	pthread_cond_t bufferNotEmpty_; // 
	pthread_t threadId_;  // 线程ID
};