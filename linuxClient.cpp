// client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

void* Send(void* arg)
{
    int* fd = static_cast<int*>(arg);
    while (1) {
        char buff[255];
        bzero(&buff, sizeof(buff));
        cin.getline(buff, 255);
        send(*fd, buff, sizeof(buff), 0);
    }
}

void* Recv(void* arg)
{
    int* fd = static_cast<int*>(arg);
    while (1) {
        char buff[255];
        int len = recv(*fd, buff, sizeof(buff), 0);

        if (len == -1) {
            perror("recv");
            break;
        }

        cout << "windows:" << buff << endl;

        bzero(&buff, sizeof(buff));
    }
}

int main()
{
    // 1. 创建通信的套接字
    cout << "++++++++++++++" << "创建套接字" << endl;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(0);
    }

    // 2. 连接服务器
    cout << "++++++++++++++" << "创建用于连接的套接字" << endl;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);   // 大端端口
    inet_pton(AF_INET, "192.168.43.3", &addr.sin_addr.s_addr);

    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect");
        exit(0);
    }

    cout << "++++++++++++++" << "连接成功， 创建线程" <<endl;

    pthread_t stid;
    pthread_t rtid;
    pthread_create(&stid, nullptr, Send, &fd);
    pthread_create(&rtid, nullptr, Recv, &fd);
    pthread_join(rtid, nullptr);
    pthread_join(stid,nullptr);
    cout << "++++++++++++++" << "结束通信" <<endl;    
    close(fd);
    return 0;
}
