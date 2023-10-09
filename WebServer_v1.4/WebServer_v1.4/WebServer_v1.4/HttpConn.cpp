#include "HttpConn.h"

HttpConn::HttpConn(int fd1, sockaddr_in addr1)
	:fd(fd1), addr(addr1), sql(sqlQuery::getInstance()){}


int HttpConn::ReadRequest() 
{
	int len = 0, total = 0;
	char tmp[1024] = { 0 };//临时接收数据，然后放入buf中
	char buf[4096] = { 0 };
	while ((len = recv(fd, tmp, sizeof(tmp), 0)) > 0) {
		if (total + len < sizeof(buf)) {
			memcpy(buf + total, tmp, len);
		}
		total += len;
	}
	if ((len == -1) && (errno == EAGAIN))
	{
		//对应缓冲区的数据被读取完了
		ParseRequest(buf);  //解析请求
		printf("success!\n");
		ProcessRequest();   //处理请求
	}
	return len;
}

void HttpConn::ParseRequest(char* requestBuffer)
{
	std::string request(requestBuffer);
	std::istringstream requestStream(request);
	std::string requestLine;
	// 解析请求行
	std::getline(requestStream, requestLine);
	auto tokens = split(requestLine, ' ');
	if (tokens.size() < 3) {
		std::cout << "Invalid request line: " << requestLine << std::endl;
		return;
	}
	httpRequest.METHOD = tokens[0];
	httpRequest.URI = tokens[1];
	httpRequest.VERSION = tokens[2];
	
	if (httpRequest.METHOD == "POST")
	{
		char* bodyStart = strstr(requestBuffer, "\r\n\r\n");
		if (bodyStart != nullptr) {
			bodyStart += 4;  // 跳过 "\r\n\r\n"
			//std::cout << "Body content: " << bodyStart << std::endl;  // 这里应输出 "key=value"
			std::string Requestor(bodyStart);
			auto bodys = split(Requestor, '&');
			for (auto body : bodys)
			{
				auto separator = body.find('=');
				if (separator != std::string::npos)
				{
					std::string key = body.substr(0, separator);
					std::string value = body.substr(separator + 1);
					httpRequest.requestBodys[key] = value;
				}
			}
		}
		else {
			std::cout << "No body found" << std::endl;
		}
	}
	std::cout << "method: " << tokens[0] << std::endl;
	std::cout << "content: " << tokens[1] << std::endl;
	std::cout << "protocol: " << tokens[2] << std::endl;
	for (auto body : httpRequest.requestBodys) 
	{
		std::cout << body.first << ": " << body.second << std::endl;
	}

}


/*
void HttpConn::ParseRequest(char* buf)
{
	//读取首行
	char* p = strchr(buf, '\n');
	int indexRequest = (p - buf);
	char Request[indexRequest];
	strncpy(Request, buf, indexRequest);
	//printf("RequestFirstLine is [%s ]\n", Request);
	//解析请求    GET /a.txt     HTTP/1.1\r\n
	printf("read ok\n");
	char method[256] = "";
	char content[256] = "";
	char protocol[256] = "";
	sscanf(buf, "%[^ ] %[^ ] %[^ \r\n]", method, content, protocol);
	//printf("method :%s \n", method);
	//printf("content :%s \n", content);
	//printf("protocol :%s \n", protocol);
	httpRequest.METHOD = method;
	httpRequest.URI = content;
	httpRequest.VERSION = protocol;

	std::cout << "method: " << method << std::endl;
	std::cout << "content: " << content << std::endl;
	std::cout << "protocol: " << protocol << std::endl;
}
*/

void HttpConn::ProcessRequest() 
{
	if (strcasecmp(httpRequest.METHOD.c_str(), "get") == 0) //判断是否是get请求
	{
		//1、设置路径 ？
		std::string fileStr = httpRequest.URI.substr(httpRequest.URI.find('/') + 1);
		//std::cout << "fileStr:" << "[" << fileStr << "]" << std::endl;
		if (fileStr.c_str() == 0)
		{
			fileStr = "./";
		}
		//2、判断file是目录还是文件
		struct stat st;
		int ret = stat(fileStr.c_str(), &st);
		if (ret < 0)
		{
			printf("file not exit!\n");
			SendHeader(404, "NOT Found", GetFileType(".html"));
			SendFile("error.html");
			return;
		}
		if (S_ISDIR(st.st_mode))
		{
			//是目录
			//printf("[DIR:]\n");
			//SendHeader(200, "OK", getFileType(".html"));
			//SendDir(fileStr.c_str());
		}
		else if (S_ISREG(st.st_mode))
		{
			//是文件
			//printf("[Request File]\n");
			SendHeader(200, "OK", GetFileType(".html"));
			SendFile(fileStr.c_str());
			printf("success!\n");
		}
	}
	else if (strcasecmp(httpRequest.METHOD.c_str(), "POST") == 0) 
	{
		//如果是POST请求，就得连接MySQL，因为整个httpConn类应该是对应一个MySql类就行了
		if (sql.identityVerify(httpRequest.requestBodys["nickname"], httpRequest.requestBodys["password"]))
		{
			SendHeader(200, "OK", GetFileType(".html"));
			SendFile("login_in.html");
			printf("success login!\n");
		}
		else 
		{
			printf("wrong name or passwd!\n");
		}
	}
	
}

void HttpConn::SendHeader(int statusCode, const char* descr, const char* type) 
{
	std::string buf1;

	buf1 = "HTTP/1.1 " + std::to_string(statusCode) + " " + descr + "\r\n";
	send(fd, buf1.c_str(), buf1.length(), 0);

	buf1 = "Content-Type:" + (std::string)type + "\r\n";
	send(fd, buf1.c_str(), buf1.length(), 0);
	//发送空行
	send(fd, "\r\n", 2, 0);
}

int HttpConn::SendFile(const char* filename)
{
	int fdFile = open(filename, O_RDONLY);

	int flag_send = 0;
	if (fdFile < 0)
	{
		perror("");
		return -1;
	}
	char buf[2048] = "";
	int len = 0;
	while (1)
	{

		len = read(fdFile, buf, sizeof(buf));
		if (len < 0)
		{
			perror("read");
			break;
		}
		else if (len == 0)
		{
			break;
		}
		else
		{
			flag_send = send(fd, buf, len, 0);
			if (flag_send < 0)
			{
				printf("sendfile error ");
			}
		}
	}
	close(fdFile);
	close(fd);
	return 0;
}

int HttpConn::GetFd()
{
	return fd;
}

void HttpConn::CloseFd()
{
	close(fd);
}

const char* HttpConn::GetFileType(const char* name)
{
	// 从右往左找，找后缀名
	const char* dot = strrchr(name, '.');
	if (dot == NULL) {
		return "text/plain; charset=utf-8";
	}
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) {
		return "text/html; charset=utf-8\r\n";
	}
	if (strcmp(dot, ".txt") == 0) {
		return "text/plain; charset=utf-8";
	}
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) {
		return "image/jpeg";
	}
	if (strcmp(dot, ".png") == 0) {
		return "image/png";
	}
	if (strcmp(dot, ".gif") == 0) {
		return "image/gif";
	}
	if (strcmp(dot, ".mkv") == 0) {
		return "video/x-matroska";
	}
	if (strcmp(dot, ".mp4") == 0) {
		return "audio/mp4";
	}
	if (strcmp(dot, ".mp3") == 0) {
		return "audio/mp3";
	}
	if (strcmp(dot, ".csv") == 0) {
		return "text/csv";
	}
	if (strcmp(dot, ".wbmp") == 0) {
		return "image/vnd.wap.wbmp";
	}
	return "text/plain; charset=utf-8";

}

std::vector<std::string> HttpConn::split(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

