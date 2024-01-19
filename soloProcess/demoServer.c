#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <error.h>

#define SERVER_PORT 8888
#define MAX_LISTEN 128
#define LOCAL_IPADDRESS "172.23.232.7"
#define BUFFER_SIZE 128

// void sigHander(int sigNum)
// {
//     int ret = 0;
//     /* 资源回收 */
//     /* todo... */
// }

int main()
{
    /* 信号注册 */
    // signal(SIGINT, sigHander);
    // signal(SIGQUIT, sigHander);
    // signal(SIGTSTP, sigHander);

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* 绑定 */
#if 0
    /* 这个结构体不好用 */
    struct sockaddr localAddress;
#else
    struct sockaddr_in localAddress;
#endif
    /* 清除脏数据 */
    memset(&localAddress, 0, sizeof(localAddress));

    /* 地址族 */
    localAddress.sin_family = AF_INET;
    /* 端口需要转成大端 */
    localAddress.sin_port = htons(SERVER_PORT);
    /* ip地址需要转成大端 */

    /* Address to accept any incoming messages.  */
    /* INADDR_ANY = 0x00000000 */
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    /*设置端口复用*/
    int enableopt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableopt, sizeof(enableopt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    ret = bind(sockfd, (struct sockaddr *)&localAddress, sizeof(localAddress));
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    /* 客户的信息 */
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));

    socklen_t clientAddressLen = 0;
    int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (acceptfd == -1)
    {
        perror("accpet error");
        exit(-1);
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    char replyBuffer[BUFFER_SIZE];
    memset(replyBuffer, 0, sizeof(replyBuffer));

    int readBytes = 0;
    while (1)
    {
        readBytes = read(acceptfd, (void *)&buffer, sizeof(buffer));
        if (readBytes <= 0)
        {
            /* todo... 资源问题 */
            printf("1111\n");
            perror("read error");
            sleep(2);
            close(acceptfd);
            break;
        }
        else
        {
            printf("client message:%s\n", buffer);
            if (strncmp(buffer, "123456", strlen("123456")) == 0)
            {
                strncpy(replyBuffer, "666", sizeof(replyBuffer) - 1);
                sleep(1);
                write(acceptfd, replyBuffer, sizeof(replyBuffer));
            }
            else if (strncmp(buffer, "778", strlen("123456")) == 0)
            {
                strncpy(replyBuffer, "889", sizeof(replyBuffer) - 1);
                sleep(1);
                write(acceptfd, replyBuffer, sizeof(replyBuffer));
            }

            sleep(3);
        }
    }
    /*关闭文件描述符*/
    close(sockfd);

    return 0;
}