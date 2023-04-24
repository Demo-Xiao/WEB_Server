#include "serverEpoll.h"
//ʹ��epoll��������ͨ�ŵ�һ������
//
//1���������ڼ������׽��֣�����Ӧ�˿ڲ��Ҽ���
int SocketBindListen()
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd < -1)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	//���ö˿ڸ���
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int ret = bind(lfd, (sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("bind");
		return -1;
	}

	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		close(lfd);
		return -1;
	}

	return lfd;
}
// �������������ӵ��ļ���ʶ����ӵ�epoll�������
int epoll_add(int fd, int epfd, struct epoll_event& ev)
{
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	if (ret < 0) 
	{
		perror("epoll_ctl");
		return -1;
	}
	return 1;
}

int Connect_create(int epfd, int lfd, epoll_event events)
{
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	//����Զ�̼�������������󣬽�������ͻ���֮���ͨ�����ӡ�
	int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &len);
	if (cfd == -1)
	{
		perror("accept");
		return -1;
	}
	//��ӡԶ�̿ͻ����ľ�����Ϣ
	char ip[16] = "";
	printf("Client:IP_address: %s, Port: %d\n",
		inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)),
		ntohs(cliaddr.sin_port));

	//��������������ӵ��������
	epoll_add(cfd, epfd, events);
	return 1;
}

int epollRun(ThreadsPool *pool)
{
	int epfd = epoll_create(1);
	if (epfd < 0)
	{
		printf("fail to epoll create!");
		return -1;
	}

	int lfd = SocketBindListen();
	if (lfd < 0)
	{
		printf("faile to SocketBindListen");
		return -1;
	}

	// �������ڵ�	
	struct epoll_event ev;
	int ret = epoll_add(lfd, epfd, ev);
	if (ret < 0)
	{
		perror("epoll_ctl");
		return -1;
	}
	epoll_event* Possible_evs = new epoll_event[Possible_evs_Length];
	while (true) 
	{
		int number = epoll_wait(epfd, Possible_evs, Possible_evs_Length, -1);
		if (number < 0)
		{
			printf("epoll_wait fail!");
			break;
		}
		for (int i = 0; i < number; i++)
		{
			if (Possible_evs[i].data.fd == lfd)
			{
				int FlagConnect = Connect_create(epfd, lfd, Possible_evs[i]);
				if (FlagConnect < 0)
				{
					printf("something wrong with connect!");
					exit(0);
				}
			}
			else if (Possible_evs[i].events & EPOLLIN)
			{
				//��Ӧ����cfd�ı仯�������Ƕ��¼�

				ForParameter *Par = new ForParameter;
				Par->fd = epfd;
				Par->ev = Possible_evs[i];
				//����һ����Ų����ṹ��

				Task* task = new Task;
				task->arg = Par;
				task->function = processWork;
				//����һ���������Ľṹ�壬�����ɺ����Ͳ������ɣ�����������Ľṹ��
				
				pool->addTask(task);
				//ReadClientRequest(epfd, &Possible_evs[i]);
				//�޸�Ϊʹ���̳߳�
			}
		}
	}
}

//��������
HttpRequest parseRequest(char* buf)
{
	//��ȡ����
	HttpRequest httprequest;
	char* p = strchr(buf, '\n');
	int indexRequest = (p - buf);
	char Request[indexRequest];
	strncpy(Request, buf, indexRequest);
	printf("RequestFirstLine is [%s ]\n", Request);
	//��������    GET /a.txt     HTTP/1.1\r\n
	printf("read ok\n");
	char method[256] = "";
	char content[256] = "";
	char protocol[256] = "";
	sscanf(buf, "%[^ ] %[^ ] %[^ \r\n]", method, content, protocol);
	printf("method :%s \n", method);
	printf("content :%s \n", content);
	printf("protocol :%s \n", protocol);
	httprequest.METHOD = method;
	httprequest.CONTENT = content;
	httprequest.PROTOCOL = protocol;
	return httprequest;
}

void processRequest(HttpRequest Request, struct epoll_event* ev)
{
	if (strcasecmp(Request.METHOD.c_str(), "get") == 0) //�ж��Ƿ���get����
	{
		//1������·�� ��
		std::string fileStr = Request.CONTENT.substr(Request.CONTENT.find('/') + 1);
		std::cout << "fileStr:" << "[" << fileStr << "]" << std::endl;
		if (fileStr.c_str() == 0)
		{
			fileStr = "./";
		}
		//2���ж�file��Ŀ¼�����ļ�
		struct stat st;
		int ret = stat(fileStr.c_str(), &st);
		if (ret < 0) 
		{
			printf("file not exit!\n");
			sendHeader(ev, 404, "NOT Found", getFileType(".html"));
			sendFile("error.html", ev->data.fd);
			return ;
		}
		if (S_ISDIR(st.st_mode)) 
		{
			//��Ŀ¼
			printf("[DIR:]\n");
			sendHeader(ev, 200, "OK", getFileType(".html"));
			sendDir(fileStr.c_str(), ev->data.fd);
		}else if (S_ISREG(st.st_mode)) 
		{
			//���ļ�
			printf("[FILE:]\n");
			sendHeader(ev, 200, "OK", getFileType(".html"));
			sendFile(fileStr.c_str(), ev->data.fd);
		}
		
	}
}


// ����Ŀ¼
int sendDir(const char* dirName, int cfd) {
	char buf[4096] = { 0 };
	sprintf(buf,
		"<html><head><title>%s</title></head><body><table>",
		dirName);


	struct dirent** nameList;
	int num = scandir(dirName, &nameList, NULL, alphasort);
	for (int i = 0; i < num; i++) {
		char* name = nameList[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName, name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode)) {
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		else {
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		send(cfd, buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		free(nameList[i]);
	}
	sprintf(buf, "</table></body></html>");
	send(cfd, buf, strlen(buf), 0);
	free(nameList);

	return 0;
}

void sendHeader(epoll_event* ev, int statusCode, const char *descr, const char *type)
{
	std::string buf1;

	buf1 = "HTTP/1.1 " + std::to_string(statusCode) + " " + descr + "\r\n";
	send(ev->data.fd, buf1.c_str(), buf1.length(), 0);

	buf1 = "Content-Type:" + (std::string)type + "\r\n";
	send(ev->data.fd, buf1.c_str(), buf1.length(), 0);
	//���Ϳ���
	send(ev->data.fd, "\r\n", 2, 0);
}

int sendFile(const char* filename, int cfd) 
{
	int fd = open(filename, O_RDONLY);
	
	int flag_send = 0;
	if (fd < 0)
	{
		perror("");
		return -1;
	}
	char buf[1024] = "";
	int len = 0;
	while (1)
	{
		
		len = read(fd, buf, sizeof(buf));
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
			flag_send = send(cfd, buf, len, 0);
			if (flag_send < 0)
			{
				printf("sendfile error ");
			}
		}
	}
	close(cfd);
	//�ر�cfd,����

	return 0;

}


const char* getFileType(const char* name)
{
	// ���������ң��Һ�׺��
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


void processWork(void* arg) 
{
	ForParameter* ProArg = (ForParameter*)arg;
	ReadClientRequest(ProArg->fd, &ProArg->ev);
}

void ReadClientRequest(int epfd, struct epoll_event* ev)
{

	//������˵Ĵ���ʹ��recv����ȡ���Ƿ������ģ�����ȷ������ȡ��ɣ�Ȼ�󻹼���ѭ�������˸о����������٣�����Ҫ���ǣ�д����Ҳ���Ǻܷ��㣬�����Ȳ��޸ģ����������
	char buf[2048] = "";
	printf("TIPS! The Value lenght is %d\n", sizeof(buf));
	int n = read(ev->data.fd, buf, sizeof(buf));
	printf("TIPS! The Value N is %d\n", n);
	if (n < 0)
	{
		perror("read");
		epoll_ctl(epfd, EPOLL_CTL_DEL, ev->data.fd, ev);
		return;
	}
	else if (n == 0)
	{
		printf("client close\n");
		epoll_ctl(epfd, EPOLL_CTL_DEL, ev->data.fd, ev);
		return;
	}
	else
	{
		HttpRequest Request =  parseRequest(buf);
		processRequest(Request, ev);
	}
}