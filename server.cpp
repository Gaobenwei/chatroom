/*简易仿写chat_server.cpp*/
#include<stdio.h>
#include <stdlib.h>
#include<sys/stat.h>
#include <arpa/inet.h>
#include<string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#define BUF_SIZE 1024
#define EPOLL_SIZE 100
#define CLIENT_SIZE 100 //在线人数

//客户端SOCKET 封装尝试
struct Client
{
	int socket;
	char ip[16]{}; //客户端IP

	bool isServerReceived;//服务端是否已经接收了一个包
    bool isClientReceived;//客户端是否已经接收了一个包

    int upFile;//上传文件描述符
    int64_t receivedBytes;//已接收字节数
    int64_t totalRBytes;//总计需要接收字节数

    int downFile;//下载文件描述符
    int64_t sentBytes;//已发送字节数
    int64_t totalSBytes;//总计需要发送字节数
};
int clnt_cnt{0};
Client clnt_socks[CLIENT_SIZE];

//连接后会接受到两类信息： 消息信息 ，以及传输的文件
/*************************协议定义******************************/
//包协议
//systemInfo:
//0x01:客户端接收包完成
//0x02:服务端接收包完成
//0x03:服务端接收文件完成
//0x04:服务端发送文件完成
//0x05:清空客户端文件列表
enum class PackType:int{
    msg = 1,//普通消息
    systemInfo = 2,//系统消息
    fileList = 3,//文件列表
    fileInfo = 4,//文件信息
    fileText = 5,//文件内容
    down = 6//下载请求
};
struct PackHeader {
    PackType type;
    int32_t bodySize;
};

//文件格式
struct FileInfo {
    char name[300];
    int64_t size;
};




//函数声明
void ErrorHandling(const char* msg);
void CloseClient(int index, int &epfd);
void HandlePacket(char* packet, ssize_t len, int index, int &epfd);
void DownFile(int index, int &epfd);

/**************临时使用的函数、变量*/
#define MAX_CLNT 256
pthread_mutex_t mutx;
void *handle_clnt(void *arg);
void send_msg(char *msg, int len);



int main(int argc,char* argv[])
{
	mkdir("./file", 0777);//创建上传文件的文件夹

    int serv_sock;
    struct sockaddr_in serv_adr;


    pthread_t t_id;
    pthread_mutex_init(&mutx,NULL);
    serv_sock=socket(PF_INET,SOCK_STREAM,0);
    if(-1==serv_sock)
    {
        ErrorHandling("socket() error");
    }
	//初始化服务器配置
    socklen_t adr_sz_serv=sizeof(serv_adr);
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
    if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
    {
        close(serv_sock);
        ErrorHandling("bind() error" );
    }
    if(listen(serv_sock,5)==-1)
    {
         close(serv_sock);
        ErrorHandling("listen() error");
    }

	//EPOLL初始化
	int epfd{};
	int ep_count{};
	struct epoll_event *ep_events;
	struct epoll_event event;
	epfd=epoll_create(EPOLL_SIZE);
	ep_events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	memset(&event,0,sizeof(event));
	event.events = EPOLLIN; //需要读取数据的情况
	event.data.fd=serv_sock;
	epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event);

	//主循环
    
    while (1)
    {
		ep_count=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
		if(-1==ep_count)
		{
			fputs("epoll_wait() error",stderr);
            break;
		}

		for(int i=0;i<EPOLL_SIZE;i++)
		{
			if(ep_events[i].data.fd==serv_sock) ////客户端请求连接时
			{
				//添加新的客户端连接
				sockaddr_in clnt_adr;
				socklen_t clnt_adr_sz=sizeof(clnt_adr);
				int clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
				if(clnt_cnt+1>CLIENT_SIZE)
				{
					fputs("人数已达到最大人数\n",stdout);
					close(clnt_sock);
					continue;
				}
				clnt_socks[clnt_cnt].socket=clnt_sock;
				char* ip=inet_ntoa(clnt_adr.sin_addr);
				memset(clnt_socks[clnt_cnt].ip,0,sizeof(clnt_socks[clnt_cnt].ip));
				memcpy(clnt_socks[clnt_cnt].ip,ip,strlen(ip));

				//将对应的客户端读事件写入epoll
				event.events=EPOLLIN;
				event.data.fd=clnt_sock;
				epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_socks[clnt_cnt].socket,&event);
				printf("[%s] connected!\n", clnt_socks[clnt_cnt].ip);
                clnt_cnt++;
			}
			else //是客户端套接字时
			{
				//获取客户端在列表中的位置
				int index{0};
				for(index=0;index<clnt_cnt;index++)
				{
					if(clnt_socks[index].socket==ep_events[i].data.fd)
					{
						break;
					}
				}

				char recvBuf[1024]{};
				//检查客户端输入事件
				if(ep_events[i].events & EPOLLIN)
				{
					//查看clnt_sock输入缓冲区中是否含有一个包头大小的数据
					PackHeader header{};
					ssize_t len=recv(ep_events[i].data.fd,&header,sizeof(PackHeader),MSG_PEEK | MSG_DONTWAIT);//设置MSG_PEEK标志来预览数据而不将其从缓冲区中移除。

					if(len<=0)
					{
						//删除epoll通知,关闭客户端
                        CloseClient(index, epfd);
					}
					else if(len==sizeof(PackHeader))
					{
						//查看clnt_sock输入缓冲区中是否含有一个包大小的数据
						len=recv(ep_events[i].data.fd,recvBuf,sizeof(PackHeader)+header.bodySize,MSG_PEEK | MSG_DONTWAIT);
						if(len==-1)
						{
							CloseClient(index,epfd);
							break;
						}
						if(len!=(long)sizeof(PackHeader)+header.bodySize)
						{//一个包没读完
							continue;
						}

						//从输入缓冲区中读出一个包
						len=read(ep_events[i].data.fd,recvBuf,sizeof(PackHeader)+header.bodySize);
						if(len==-1)
						{
							CloseClient(index,epfd);
							fputs("buf rread",stderr);
							break;
						}
						//处理包
						PackHeader*head=(PackHeader*)recvBuf;
						printf("[%s]packet type is %d\n",clnt_socks[index].ip,head->type);
						HandlePacket(recvBuf, len, index, epfd);
					}
				}

				//检查客户端输出事件
				if(ep_events[i].events & EPOLLOUT)
				{
					DownFile(index,epfd);
				}
			}
		}       
    }
	//释放
	free(ep_events);
    close(epfd);
	close(serv_sock);
    return 0;
}

void ErrorHandling(const char *msg)
{
    fputs(msg,stderr);
    fputc('\n',stderr);
    exit(0);
}

void CloseClient(int index, int &epfd)
{
	char ip[16]{};
	memcpy(ip,clnt_socks[index].ip,sizeof(clnt_socks[index].ip));
	int sock=clnt_socks[index].socket;
	//判断该客户端是否在传输文件,需接受上传，需发送不下载

	if(clnt_socks[index].receivedBytes!=clnt_socks[index].totalRBytes)
	{
		close(clnt_socks[index].upFile);
	}
	if(clnt_socks[index].sentBytes!=clnt_socks[index].totalSBytes)
	{
		close(clnt_socks[index].downFile);
	}

	//从列表中删除当前客户端
	while (index<clnt_cnt-1)
	{
		clnt_socks[index]=clnt_socks[index+1];
		index++;
	}
	clnt_cnt--;
	//删除epoll通知
	epoll_ctl(epfd,EPOLL_CTL_DEL,sock,NULL);
	close(sock);
	printf("[%s] disconnected!\n", ip);
}

void HandlePacket(char* packet, ssize_t len, int index, int &epfd)
{
	PackHeader* pHeader=(PackHeader*)packet;

	if (pHeader->type==PackType::msg)
	{
		char *pInfo=(char*)(packet+sizeof(PackHeader));
		printf("[%s] msg is %s \n",clnt_socks[index].ip,pInfo);
		for(int i=0;i<clnt_cnt;i++)
		{
			write(clnt_socks[i].socket,packet,len);
		}
	}
	else if(pHeader->type == PackType::systemInfo)
	{
		char *pInfo = (char*)(packet + sizeof(PackHeader));
        printf("[%s] recv info is %d\n", clnt_socks[index].ip, *pInfo);
		//0x01:客户端接收包完成
        //0x02:服务端接收包完成
        //0x03:服务端接收文件完成
        //0x04:服务端发送文件完成
		if(*pInfo==0x01)
		{
			clnt_socks[index].isClientReceived=true;
		}
		else if (*pInfo == 0x02) {
        }
        else if (*pInfo == 0x03) {
        }
        else if (*pInfo == 0x04) {
        }
        else if (*pInfo == 0x05) {
        }
	}
	else if(pHeader->type == PackType::fileList)
	{
		//向客户端发送清空列表完成标志
        //系统消息包体
		char body=0x05;
		//系统消息包头
		PackHeader header{};
		header.type=PackType::systemInfo;
		header.bodySize=sizeof(body);
		//组织系统消息包
		char *packet=new char[sizeof(PackHeader)+sizeof(body)];
		memcpy(packet,&header,sizeof(PackHeader));
		memcpy(packet+sizeof(PackHeader),&body,sizeof(body));
		write(clnt_socks[index].socket,packet,sizeof(PackHeader)+sizeof(body));
		delete[] packet;
		packet=nullptr;

		//获取./file/目录下所有的普通文件
		DIR* dir=opendir("./file"); //打开目录
		if(dir==nullptr)
		{
			return;
		}
		dirent* entry;
		char *name=nullptr;
		while((entry=readdir(dir))!=NULL)
		{
			if(entry->d_type==DT_REG) //DT_REG：普通文件
			{
				//文件列表包体
				name=entry->d_name;
				printf("[%s]file list:%s\n", clnt_socks[index].ip, name);
				//文件列表包头
				memset(&header,0,sizeof(PackHeader));
				header.type=PackType::fileList;
				header.bodySize=(int32_t)strlen(name);

				//组织文件列表包
				packet=new char[sizeof(PackHeader)+strlen(name)];
				memcpy(packet,&header,sizeof(PackHeader));
				memcpy(packet+sizeof(PackHeader),name,strlen(name));

				write(clnt_socks[index].socket,packet,sizeof(PackHeader) + strlen(name));
				delete[] packet;
				packet=nullptr;
			}
		}
	}
	else if(pHeader->type == PackType::fileInfo)
	{
		//获取要接收的文件信息
		FileInfo* pFileInfo=(FileInfo*)(packet+sizeof(PackHeader));
		clnt_socks[index].totalRBytes=pFileInfo->size;
		clnt_socks[index].receivedBytes=0;
		
		//设置要保存的路径
		char path[PATH_MAX] = {};
		sprintf(path, "./file/%s", pFileInfo->name);
        printf("[%s]recv file is %s, total is %d\n", clnt_socks[index].ip, path, clnt_socks[index].totalRBytes);
		//若文件存在则删除
		if (access(path, F_OK) == 0)
            unlink(path);
		//打开文件
		clnt_socks[index].upFile=open(path, O_CREAT | O_WRONLY, 0777);
	}
	else if(pHeader->type == PackType::fileText)
	{
		//写入接收的文件内容
		len = write(clnt_socks[index].upFile, packet + sizeof(PackHeader), pHeader->bodySize);
        if (len == -1) {
            CloseClient(index, epfd);
            return;
        }
		//向clnt_sock发送包接收完成标志
        //系统消息包体
		char body = 0x02;
		//系统消息包头
		PackHeader header = {};
        header.type = PackType::systemInfo;
        header.bodySize = sizeof(body);
		//组织系统消息包
		char* packet = new char[sizeof(PackHeader) + sizeof(body)];
        memcpy(packet, &header, sizeof(PackHeader));
        memcpy(packet + sizeof(PackHeader), &body, sizeof(body));
        write(clnt_socks[index].socket, packet, sizeof(PackHeader) + sizeof(body));
        delete[] packet;
        packet = nullptr;

		//统计已接受字节
		clnt_socks[index].receivedBytes+=len;
		printf("[%s]upLoad(%ld/%ld)...\n", clnt_socks[index].ip, clnt_socks[index].receivedBytes, clnt_socks[index].totalRBytes);
		//若全部接收完成，关闭文件
		if(clnt_socks[index].receivedBytes==clnt_socks[index].totalRBytes)
		{
			close(clnt_socks[index].upFile);

			//向客户端发送服务端接收文件完成信息
			//系统消息包体
            body = 0x03;
            //系统消息包头
            memset(&header, 0, sizeof(PackHeader));
            header.type = PackType::systemInfo;
            header.bodySize = sizeof(body);
            //组织系统包
			packet = new char[sizeof(PackHeader) + sizeof(body)];
            memcpy(packet, &header, sizeof(PackHeader));
            memcpy(packet + sizeof(PackHeader), &body, sizeof(body));
			write(clnt_socks[index].socket,packet,sizeof(PackHeader)+sizeof(body));
			delete[] packet;
            packet = nullptr;

            printf("[%s]file upLoad ok\n", clnt_socks[index].ip);
		}
	}
	else if(pHeader->type == PackType::down)
	{
		//获取要下载的文件信息
		FileInfo fileInfo = {};
        char* fileName = (char*)(packet + sizeof(PackHeader));
        sprintf(fileInfo.name, "./file/%s", fileName);//获取下载路径
        printf("[%s]down file is %s\n", clnt_socks[index].ip, fileInfo.name);
        //获取文件大小
		struct stat fileStat;
        if (stat(fileInfo.name, &fileStat) == 0) {
            fileInfo.size = fileStat.st_size;
            clnt_socks[index].totalSBytes = fileStat.st_size;
            clnt_socks[index].sentBytes = 0;
        }
        else {
            //文件不存在
            printf("[%s]file not exist\n",clnt_socks[index].ip);
            return;
        }
		//向客户端发送待下载文件的信息
        //文件信息包头
        PackHeader header = {};
        header.type = PackType::fileInfo;
        header.bodySize = sizeof(FileInfo);
        //组织文件信息包
        char* packet = new char[sizeof(PackHeader) + sizeof(FileInfo)];
        memcpy(packet, &header, sizeof(PackHeader));
        memcpy(packet + sizeof(PackHeader), &fileInfo, sizeof(FileInfo));
        write(clnt_socks[index].socket, packet, sizeof(PackHeader) + sizeof(FileInfo));
        delete[] packet;
        packet = nullptr;

		struct epoll_event event;
		memset(&event,0,sizeof(event));
		//增加该客户端epoll写事件
        event.events = EPOLLIN | EPOLLOUT;
        event.data.fd = clnt_socks[index].socket;
        epoll_ctl(epfd, EPOLL_CTL_MOD, clnt_socks[index].socket, &event);

        //以只读方式打开待下载的文件
        clnt_socks[index].downFile = open(fileInfo.name, O_RDONLY);
        clnt_socks[index].isClientReceived = true;
	}
}


void DownFile(int index,int &epfd) 
{
	//客户端未接收完成一个包就不要发送下一个包
    if (clnt_socks[index].isClientReceived != true)
        return;
   
    //客户端已接收完成上一个包，开始传输下一个包
    //文件内容包体，读取文件信息
    char body[1024 * 10] = {};
    ssize_t len = read(clnt_socks[index].downFile, body, sizeof(body));
    if (len == -1) {
        CloseClient(index, epfd);
        return;
    }
    //文件内容包头
    PackHeader header = {};
    header.type = PackType::fileText;
    header.bodySize = (int32_t)len;
    //组织文件内容包
    char* packet = new char[sizeof(PackHeader) + len];
    memcpy(packet, &header, sizeof(PackHeader));
    memcpy(packet + sizeof(PackHeader), body, len);
    write(clnt_socks[index].socket, packet, sizeof(PackHeader) + len);
    delete[] packet;
    packet = nullptr;
    clnt_socks[index].isClientReceived = false;

    //判断文件是否发送完成
    clnt_socks[index].sentBytes += len;
    printf("[%s]downing(%ld/%ld)...\n", clnt_socks[index].ip, clnt_socks[index].sentBytes, clnt_socks[index].totalSBytes);
    if (clnt_socks[index].sentBytes == clnt_socks[index].totalSBytes) {    
        //向客户端发送文件发送完成包
        //系统消息包体
        char info = 0x04;
        //系统消息包头
        memset(&header, 0, sizeof(PackHeader));
        header.type = PackType::systemInfo;
        header.bodySize = sizeof(info);
        //组织系统消息包
        packet = new char[sizeof(PackHeader) + sizeof(info)];
        memcpy(packet, &header, sizeof(PackHeader));
        memcpy(packet + sizeof(PackHeader), &info, sizeof(info));
        write(clnt_socks[index].socket, packet, sizeof(PackHeader) + sizeof(info));
        delete[] packet;
        packet = nullptr;

		struct epoll_event event;
        //传输完成，关闭文件，解除epoll写入事件
        event.events = EPOLLIN;
        event.data.fd = clnt_socks[index].socket;
        epoll_ctl(epfd, EPOLL_CTL_MOD, clnt_socks[index].socket, &event);

        close(clnt_socks[index].downFile);
        printf("[%s]file download ok\n", clnt_socks[index].ip);
    }
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <pthread.h>

// #define BUF_SIZE 100
// #define MAX_CLNT 256

// void * handle_clnt(void * arg);
// void send_msg(char * msg, int len);
// void error_handling(char * msg);

// int clnt_cnt=0;
// int clnt_socks[MAX_CLNT];
// pthread_mutex_t mutx;

// int main(int argc, char *argv[])
// {
// 	int serv_sock, clnt_sock;
// 	struct sockaddr_in serv_adr, clnt_adr;
// 	socklen_t clnt_adr_sz;
// 	pthread_t t_id;
// 	if(argc!=2) {
// 		printf("Usage : %s <port>\n", argv[0]);
// 		exit(1);
// 	}
  
// 	pthread_mutex_init(&mutx, NULL);
// 	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

// 	memset(&serv_adr, 0, sizeof(serv_adr));
// 	serv_adr.sin_family=AF_INET; 
// 	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
// 	serv_adr.sin_port=htons(atoi(argv[1]));
	
// 	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
// 		error_handling("bind() error");
// 	if(listen(serv_sock, 5)==-1)
// 		error_handling("listen() error");
	
// 	while(1)
// 	{
// 		clnt_adr_sz=sizeof(clnt_adr);
// 		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
// 		pthread_mutex_lock(&mutx);
// 		clnt_socks[clnt_cnt++]=clnt_sock;
// 		pthread_mutex_unlock(&mutx);
	
// 		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
// 		pthread_detach(t_id);
// 		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
// 	}
// 	close(serv_sock);
// 	return 0;
// }
	
// void * handle_clnt(void * arg)
// {
// 	int clnt_sock=*((int*)arg);
// 	int str_len=0, i;
// 	char msg[BUF_SIZE];
	
// 	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
// 		send_msg(msg, str_len);
	
// 	pthread_mutex_lock(&mutx);
// 	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
// 	{
// 		if(clnt_sock==clnt_socks[i])
// 		{
// 			while(i++<clnt_cnt-1)
// 				clnt_socks[i]=clnt_socks[i+1];
// 			break;
// 		}
// 	}
// 	clnt_cnt--;
// 	pthread_mutex_unlock(&mutx);
// 	close(clnt_sock);
// 	return NULL;
// }
// void send_msg(char * msg, int len)   // send to all
// {
// 	int i;
// 	pthread_mutex_lock(&mutx);
// 	for(i=0; i<clnt_cnt; i++)
// 		write(clnt_socks[i], msg, len);
// 	pthread_mutex_unlock(&mutx);
// }
// void error_handling(char * msg)
// {
// 	fputs(msg, stderr);
// 	fputc('\n', stderr);
// 	exit(1);
// }
