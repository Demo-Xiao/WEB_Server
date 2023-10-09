#include "WebServer.h"

WebServer::WebServer(int port,const std::string& fileName)
	:Port(port), Pool(new ThreadsPool(10)), log(fileName)
{
	//Pool->threadInit();
	myEpoll = new EpollObj(1024);
	//EPOLLRDHUP | EPOLLET; 
	listenEvent = EPOLLIN | EPOLLET; // 设置监听的工作模式   ET or LT    此处的EPOLLRDHUP也需要修改
	listenFd = SocketBindListen();
	myEpoll->EpollAdd(listenFd, listenEvent);
	Pool->start(5);
	log.start();
}
WebServer::~WebServer()
{
	delete myEpoll;
	delete Pool;
}


int WebServer::SocketBindListen()
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		return -1;
	}

	// 设为非阻塞模式
	int ret = SetSocketNonBlocking(lfd);
	if (ret < 0)
	{
		perror("Set non block failed!");
		return -1;
	}


	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = INADDR_ANY;

	//设置端口复用
	int opt = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
	{
		perror("setsocketopt");
	}
	
	ret = bind(lfd, (sockaddr*)&addr, sizeof(addr));
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

void WebServer::Start()
{
	log << "---WEBSERVER  startup!!!---";
	while (true) 
	{
		int numEvents = myEpoll->EpollWait();
		for (int i = 0; i < numEvents; i++) 
		{
			int fd = myEpoll->GetEventFd(i);
			uint32_t events = myEpoll->GetEvents(i);
			if (fd == listenFd) 
			{
				//对应新的连接
				DealListen();
			}
			else if (events & EPOLLIN) 
			{ 
				//cfd的变化，对应读事件
				assert(clients.count(fd) > 0);
				std::function<void()> task = std::bind(&WebServer::DealRead, this, clients[fd]);
				log << "A new ReadEvent!";
				Pool->addTask(task);
			}
		}
	}
}

void WebServer::DealListen()
{
	struct sockaddr_in cliaddr;   // 表示socket地址的
	socklen_t len = sizeof(cliaddr);
	//接受远程计算机的连接请求，建立起与客户机之间的通信连接。
	int cfd = accept(listenFd, (struct sockaddr*)&cliaddr, &len);
	if (cfd == -1)
	{
		perror("accept");
		return;
	}
	AddClient(cfd, cliaddr); 
}
void WebServer::DealRead(HttpConn* client) 
{
	assert(client);
	int flagRead = client->ReadRequest();
	if (flagRead == 0 && (errno != EAGAIN))
	{
		DeleteClient(client);
		//std::cout << "Connection close!" << std::endl;
		printf("Connection close (Manual shutdown)\n");
	}
	/*
	else if (flagRead == -1 && errno == EAGAIN)
	{
		DeleteClient(client);
		printf("Connection close (Automatic shutdown)\n");
	}*/

}




void WebServer::AddClient(int fd, sockaddr_in addr)
{
	//打印远程客户机的具体信息
	char ip[16] = "";
	printf("Client:IP_address: %s, Port: %d\n",
		inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip)),
		ntohs(addr.sin_port));
	log << "A new Client is connected!";
	clients[fd] = new HttpConn(fd, addr);
	myEpoll->EpollAdd(fd, EPOLLIN | EPOLLET);
	SetSocketNonBlocking(fd);
}

void WebServer::DeleteClient(HttpConn* client)
{
	int fd = client->GetFd();
	myEpoll->EpollDelete(fd);
	client->CloseFd();
	clients.erase(fd);
}





int	WebServer::SetSocketNonBlocking(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1)
		return -1;

	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flag) == -1)
		return -1;
	return 0;
}



