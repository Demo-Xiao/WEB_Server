#include "FileUtil.h"

AppendFile::AppendFile(std::string filename)
	:fp_(fopen(filename.c_str(), "a+")), writtenBytes_(0)
{
	setbuffer(fp_, buffer_, sizeof buffer_);
	assert(fp_);
}

AppendFile::~AppendFile() 
{
	fclose(fp_); // 关闭文件指针
}

void AppendFile::append(const char* logInfo, const size_t len) 
{
	size_t written = 0;
	int i = 0;
	while (written != len) 
	{
		size_t remain = len - written;

		/*
		printf("%d\n", i);
		printf("%d\n", len);
		printf("%d\n", written);
		printf("%lu\n", remain);
		printf("%s\n", " :  begin to write!");
		*/


		i++;
		size_t n = write(logInfo + written, remain);
		if (n != remain) 
		{
			int err = ferror(fp_);
			if (err) 
			{
				//fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
				clearerr(fp_); // clear error indicators for fp_
				break;
			}
		}
		written += n;
	}
	writtenBytes_ += written;
	//size_t n = this->write(logInfo, len); // 成功写入的字节数
	/*size_t remain = len - n;
	while (remain > 0)
	{
		size_t x = this->write(logInfo + n, remain);
		if (x == 0) 
		{
			int err = ferror(fp_);
			if (err) fprintf(stderr, "AppendFile::append() failed !\n");
			break;
		}
		n += x;
		remain = len - n;
	}*/
}

void AppendFile::flush() 
{
	fflush(fp_);
}

size_t AppendFile::write(const char* logInfo, size_t len)
{
	return fwrite_unlocked(logInfo, 1, len, fp_);
}
