#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <error.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <ctype.h>
#define SERVER_IP "0,0,0,0"
#define PORT 8888
#define LISTEN_SIZE 128
#define BUFFER_SIZE 128
/*单进程线程实现并发*/
int main()
{

    /*套接字*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    bzero(&serverAddr, sizeof(serverAddr));
    /*memset(&serverAddr,0,sizeof(serverAddr));*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    int ret = inet_pton(AF_INET, SERVER_IP, (void *)&serverAddr.sin_addr.s_addr);
    if (ret == -1)
    {
        perror("inet_pton error");
        exit(-1);
    }
    /*绑定*/
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, serverAddrLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }
    /*监听*/
    ret = listen(sockfd, LISTEN_SIZE);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    fd_set readSet;

    /*清空集合*/
    FD_ZERO(&readSet);

    /*将监听的文件描述符添加到读集合中*/
    FD_SET(sockfd, &readSet);

    int maxfd = sockfd;
    /*备份*/
    fd_set accordSet;
    /*清空集合*/
    FD_ZERO(&accordSet);
#if 0
    struct timeval timeValue;
    bzero(&timeValue, sizeof(timeValue));
    timeValue.tv_sec = 5;
    timeValue.tv_usec = 0;
#endif
    while (1)
    {
        /*备份读集合*/
        accordSet = readSet;
        ret = select(maxfd + 1, &accordSet, NULL, NULL, NULL);
        /*如果timeout参数不为NULL ，这里有三种情况，select出错，超时，接收到io请求*/
        if (ret == -1)
        {
            perror("listen error");
            break;
        }
        /*如果sockfd在readSet集合里面*/
        if (FD_ISSET(sockfd, &accordSet))
        {
            int acceptfd = accept(sockfd, NULL, NULL);
            if (acceptfd == -1)
            {
                perror("accept error");
                break;
            }
            /*将通信的句柄，放到读集合*/
            FD_SET(acceptfd, &readSet);
            /*更新maxfd*/
            maxfd = maxfd < acceptfd ? acceptfd : maxfd;

            /*程序到这里说明可能有通信*/
            for (int idx = 0; idx <= maxfd; idx++)
            {
                if (idx != sockfd && FD_ISSET(idx, &accordSet))
                {
                    /*程序到这里一定有通信*/
                    char buffer[BUFFER_SIZE];
                    bzero(buffer, sizeof(buffer));
                    int readBytes = read(idx, buffer, sizeof(buffer));
                    if (readBytes < 0)
                    {
                        perror("read error");
                        /*将该通信句柄从监听的集合中删除*/
                        FD_CLR(idx, &readSet);
                        /*关闭句柄*/
                        close(idx);
                        /*让下一个已经ready的fd句柄进行通信*/
                        continue;
                    }
                    else if (readBytes == 0)
                    {
                        /*客户端断开*/
                        printf("客户端断开连接\n");
                        /*将该通信句柄从监听的集合中删除*/
                        FD_CLR(idx, &readSet);
                        close(idx);
                        continue;
                    }
                    else
                    {
                        printf("client massage=%s\n", buffer);

                        for (int jdx = 0; jdx < readBytes; jdx++)
                        {
                            buffer[jdx] = toupper(buffer[jdx]);
                        }
                        write(idx, buffer, readBytes);
                        usleep(500);
                    }
                }
            }
        }
    }
    /*关闭文件描述符*/
    close(sockfd);
    return 0;
}
