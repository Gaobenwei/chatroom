#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <dirent.h>

//宏定义
#define EPOLL_SIZE 100
#define CLIENT_SIZE 100//最大在线人数

/*************************协议定义******************************/
//包协议
//systemInfo:
//0x01:客户端接收包完成
//0x02:服务端接收包完成
//0x03:服务端接收文件完成
//0x04:服务端发送文件完成
//0x05:清空客户端文件列表
enum class PackType{
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
/**************************end*********************************/
//epoll信息
struct Epoll {
    int epfd;
    epoll_event* events;
    epoll_event event;
    int count;
};
//服务端socket
struct Server {
    int socket;
    uint16_t port;
};
//客户端socket
struct Client {
    int socket;
    char ip[16];//客户端ip

    bool isServerReceived;//服务端是否已经接收了一个包
    bool isClientReceived;//客户端是否已经接收了一个包

    int upFile;//上传文件描述符
    int64_t receivedBytes;//已接收字节数
    int64_t totalRBytes;//总计需要接收字节数

    int downFile;//下载文件描述符
    int64_t sentBytes;//已发送字节数
    int64_t totalSBytes;//总计需要发送字节数
};
//客户端列表
Client clients[CLIENT_SIZE] = {};
int clientCount = 0;


//函数声明
void InitServer(Server &server);
void CloseClient(int index, Epoll& epoll);
void HandlePacket(char* packet, ssize_t len, int index, Epoll& epoll);
void DownFile(int index, Epoll& epoll);
//void InitServer(Server& server);
//void CloseClient(int index);
//void HandlePacket(char* packet, ssize_t len, int index);
//void DownFile(int index);

int main(int argc, char* argv[])
{
    mkdir("./file", 0777);//创建上传文件的文件夹

    //初始化服务器相关配置1
    Server server = {};

    server.port = 6000;
    InitServer(server);

    //epoll初始化
    Epoll epoll = {};
    epoll.epfd = epoll_create(1);
    epoll.events = new epoll_event[EPOLL_SIZE];
    epoll_event event;
    memset(&epoll.event, 0, sizeof(event));
    epoll.event.events = EPOLLIN;
    epoll.event.data.fd = server.socket;
    epoll_ctl(epoll.epfd, EPOLL_CTL_ADD, server.socket, &epoll.event);

    //主循环
    while (1) {
        epoll.count = epoll_wait(epoll.epfd, epoll.events, EPOLL_SIZE, -1);
        if (-1 == epoll.count) {
            fputs("epoll_wait error!", stderr);
            break;
        }

        for (int i = 0; i < epoll.count; i++) {
            if (epoll.events[i].data.fd == server.socket) {
                //添加新的客户端连接
                sockaddr_in addrCli;
                socklen_t addrCliLen = sizeof(addrCli);
                int sock = accept(server.socket, (sockaddr*)&addrCli, &addrCliLen);
                if (clientCount + 1 > CLIENT_SIZE) {
                    fputs("人数已到最大人数！\n", stdout);
                    close(sock);
                    continue;
                }

                clients[clientCount].socket = sock;
                char* ip = inet_ntoa(addrCli.sin_addr);
                memset(clients[clientCount].ip, 0, sizeof(clients[clientCount].ip));
                memcpy(clients[clientCount].ip, ip, strlen(ip));

                //将对应的客户端读事件写入epoll
                event.events = EPOLLIN;
                event.data.fd = clients[clientCount].socket;
                epoll_ctl(epoll.epfd, EPOLL_CTL_ADD, clients[clientCount].socket, &event);
             
                printf("[%s] connected!\n", clients[clientCount].ip);
                clientCount++;
            }
            else {
                //获取客户端在列表中的位置
                int index = 0;
                for (index = 0; index < clientCount; index++) {
                    if (clients[index].socket == epoll.events[i].data.fd)
                        break;
                }

                char recvBuf[1024] = {};
                //检查客户端输入事件
                if (epoll.events[i].events & EPOLLIN) {
                    //查看客户端输入缓冲区中是否含有一个包头大小的数据
                    PackHeader header = {};
                    ssize_t len = recv(epoll.events[i].data.fd, &header, sizeof(PackHeader), MSG_PEEK | MSG_DONTWAIT);

                    if (len <= 0) {
                        //删除epoll通知,关闭客户端
                        CloseClient(index, epoll);
                    }
                    else if (len == sizeof(PackHeader)) {
                        //查看客户端输入缓冲区中是否含有一个包大小的数据
                        len = recv(epoll.events[i].data.fd, recvBuf, sizeof(PackHeader) + header.bodySize, MSG_PEEK | MSG_DONTWAIT);
                        if (len == -1) {
                            CloseClient(index, epoll);
                            break;
                        }
                            
                        if (len != (long)sizeof(PackHeader) + header.bodySize)
                            continue;

                        //从输入缓冲区中读出一个包
                        len = read(epoll.events[i].data.fd, recvBuf, sizeof(PackHeader) + header.bodySize);
                        if (len == -1) {
                            CloseClient(index, epoll);
                            perror("buf read");
                            break;
                        }
                        //处理包
                        PackHeader* head = (PackHeader*)recvBuf;
                        printf("[%s]packet type is %d\n", clients[index].ip, head->type);
                        HandlePacket(recvBuf, len, index, epoll);
                    }
                }

                //检查客户端输出事件
                if (epoll.events[i].events & EPOLLOUT) {
                    DownFile(index, epoll);
                }
            }
        }

    }

    //销毁
    delete[] epoll.events;
    close(epoll.epfd);
    close(server.socket);
    return 0;
}

void ErrorHanding(const char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(0);
}

void InitServer(Server &server) {
    //socket
    server.socket = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == server.socket)
        ErrorHanding("server socket create error!");

    //bind
    sockaddr_in addrSrv;
    socklen_t addrSrvLen = sizeof(addrSrv);
    memset(&addrSrv, 0, sizeof(addrSrv));

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
    addrSrv.sin_port = htons(server.port);
    if (-1 == bind(server.socket, (sockaddr*)&addrSrv, addrSrvLen)) {
        close(server.socket);
        ErrorHanding("server bind error");
    }

    //listen
    if (-1 == listen(server.socket, 5)) {
        close(server.socket);
        ErrorHanding("server listen error");
    }
}

void CloseClient(int index, Epoll& epoll) {
    char ip[16] = {};
    memcpy(ip, clients[index].ip, sizeof(clients[index].ip));
    int socket = clients[index].socket;
   
    //判断该客户端是否在传输文件
    if (clients[index].receivedBytes != clients[index].totalRBytes)
        close(clients[index].upFile);
    if (clients[index].sentBytes != clients[index].totalSBytes)
        close(clients[index].downFile);
   
    //从列表中删除当前客户端
    while (index < clientCount - 1) {
        clients[index] = clients[index + 1];
        index++;
    }
    clientCount--;

    //删除epoll通知
    epoll_ctl(epoll.epfd, EPOLL_CTL_DEL, socket, NULL);
    //关闭连接
    close(socket);

    printf("[%s] disconnected!\n", ip);
}

void HandlePacket(char *packet, ssize_t len, int index, Epoll& epoll) {
    PackHeader* pHeader = (PackHeader*)packet;

    if (pHeader->type == PackType::msg) {
        char* pInfo = (char*)(packet + sizeof(PackHeader));
        printf("[%s]msg is %s\n", clients[index].ip, pInfo);
        for (int i = 0; i < clientCount; i++)
            write(clients[i].socket, packet, len);
    }
    else if (pHeader->type == PackType::systemInfo) {
        char *pInfo = (char*)(packet + sizeof(PackHeader));
        printf("[%s]recv info is %d\n", clients[index].ip, *pInfo);
        //0x01:客户端接收包完成
        //0x02:服务端接收包完成
        //0x03:服务端接收文件完成
        //0x04:服务端发送文件完成
        //0x05:清空客户端文件列表
        if (*pInfo == 0x01) {
            clients[index].isClientReceived = true;
        }
        else if (*pInfo == 0x02) {
            clients[index].isServerReceived=true;
        }
        else if (*pInfo == 0x03) {
        
        }
        else if (*pInfo == 0x04) {
        
        }
        else if (*pInfo == 0x05) {
        
        }
    }
    else if (pHeader->type == PackType::fileList) {
        //向客户端发送清空列表完成标志
        //系统消息包体
        char body = 0x05;
        //系统消息包头
        PackHeader header = {};
        header.type = PackType::systemInfo;
        header.bodySize = sizeof(body);
        //组织系统消息包
        char* packet = new char[sizeof(PackHeader) + sizeof(body)];
        memcpy(packet, &header, sizeof(PackHeader));
        memcpy(packet + sizeof(PackHeader), &body, sizeof(body));
        write(clients[index].socket, packet, sizeof(PackHeader) + sizeof(body));
        delete[] packet;
        packet = nullptr;

        //获取./file/目录下所有的普通文件
        DIR* dir = opendir("./file");
        if (dir == nullptr)
            return;
        dirent* entry;

        char* name = nullptr;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                //文件列表包体
                name = entry->d_name;
                printf("[%s]file list:%s\n", clients[index].ip, name);
                //文件列表包头
                memset(&header, 0, sizeof(PackHeader));
                header.type = PackType::fileList;
                header.bodySize = (int32_t)strlen(name);

                //组织文件列表包
                packet = new char[sizeof(PackHeader) + strlen(name)];
                memcpy(packet, &header, sizeof(PackHeader));
                memcpy(packet + sizeof(PackHeader), name, strlen(name));

                write(clients[index].socket, packet, sizeof(PackHeader) + strlen(name));
                delete[] packet;
                packet = nullptr;
            }
        }
    }
    else if (pHeader->type == PackType::fileInfo) {
        //获取要接收的文件信息
        FileInfo* pFileInfo = (FileInfo*)(packet + sizeof(PackHeader));
        clients[index].totalRBytes = pFileInfo->size;
        clients[index].receivedBytes = 0;

        //设置要保存的路径
        char path[PATH_MAX] = {};
        sprintf(path, "./file/%s", pFileInfo->name);
        printf("[%s]recv file is %s, total is %d\n", clients[index].ip, path, clients[index].totalRBytes);
        //若文件存在则删除
        if (access(path, F_OK) == 0)
            unlink(path);
        //打开文件
        clients[index].upFile = open(path, O_CREAT | O_WRONLY, 0777);
    }
    else if (pHeader->type == PackType::fileText) {
        //写入接收的文件内容
        len = write(clients[index].upFile, packet + sizeof(PackHeader), pHeader->bodySize);
        if (len == -1) {
            CloseClient(index, epoll);
            return;
        }
        
        //向客户端发送包接收完成标志
        //系统消息包体
        char body = 0x02;
        //系统消息包头
        PackHeader header = {};
        header.type = PackType::systemInfo;
        header.bodySize = sizeof(body);
        //组织系统消息包
        char* packet = new char[sizeof(PackHeader) + sizeof(body)];
        //memcpy(packet, &header, sizeof(PackHeader));
        //memcpy(packet + sizeof(PackHeader), &body, sizeof(body));
        //write(clients[index].socket, packet, sizeof(PackHeader) + sizeof(body));
        delete[] packet;
        packet = nullptr;

        //统计已接受字节
        clients[index].receivedBytes += len;
        printf("[%s]upLoad(%ld/%ld)...\n", clients[index].ip, clients[index].receivedBytes, clients[index].totalRBytes);
        //若全部接收完成，关闭文件
        if (clients[index].receivedBytes == clients[index].totalRBytes) {
            close(clients[index].upFile);

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
            write(clients[index].socket, packet, sizeof(PackHeader) + sizeof(body));
            delete[] packet;
            packet = nullptr;

            printf("[%s]file upLoad ok\n", clients[index].ip);
        }
    }
    else if (pHeader->type == PackType::down) {
        //获取要下载的文件信息
        FileInfo fileInfo = {};
        char* fileName = (char*)(packet + sizeof(PackHeader));
        sprintf(fileInfo.name, "./file/%s", fileName);//获取下载路径
        printf("[%s]down file is %s\n", clients[index].ip, fileInfo.name);
        //获取文件大小
        struct stat fileStat;
        if (stat(fileInfo.name, &fileStat) == 0) {
            fileInfo.size = fileStat.st_size;
            clients[index].totalSBytes = fileStat.st_size;
            clients[index].sentBytes = 0;
        }
        else {
            //文件不存在
            printf("[%s]file not exist\n",clients[index].ip);
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
        write(clients[index].socket, packet, sizeof(PackHeader) + sizeof(FileInfo));
        delete[] packet;
        packet = nullptr;

        //增加该客户端epoll写事件
        epoll.event.events = EPOLLIN | EPOLLOUT;
        epoll.event.data.fd = clients[index].socket;
        epoll_ctl(epoll.epfd, EPOLL_CTL_MOD, clients[index].socket, &epoll.event);

        //以只读方式打开待下载的文件
        clients[index].downFile = open(fileInfo.name, O_RDONLY);
        clients[index].isClientReceived = true;
    }
}

void DownFile(int index, Epoll& epoll) {
    //客户端未接收完成一个包就不要发送下一个包
    if (clients[index].isClientReceived != true)
        return;
   
    //客户端已接收完成上一个包，开始传输下一个包
    //文件内容包体，读取文件信息
    char body[1024 * 10] = {};
    ssize_t len = read(clients[index].downFile, body, sizeof(body));
    if (len == -1) {
        CloseClient(index, epoll);
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
    write(clients[index].socket, packet, sizeof(PackHeader) + len);
    delete[] packet;
    packet = nullptr;
    clients[index].isClientReceived = false;

    //判断文件是否发送完成
    clients[index].sentBytes += len;
    printf("[%s]downing(%ld/%ld)...\n", clients[index].ip, clients[index].sentBytes, clients[index].totalSBytes);
    if (clients[index].sentBytes == clients[index].totalSBytes) {    
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
        write(clients[index].socket, packet, sizeof(PackHeader) + sizeof(info));
        delete[] packet;
        packet = nullptr;

        //传输完成，关闭文件，解除epoll写入事件
        epoll.event.events = EPOLLIN;
        epoll.event.data.fd = clients[index].socket;
        epoll_ctl(epoll.epfd, EPOLL_CTL_MOD, clients[index].socket, &epoll.event);

        close(clients[index].downFile);
        printf("[%s]file download ok\n", clients[index].ip);
    }
}
