#include <iostream>
#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <string>
#include <thread>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

void Recv(int fd)
{
    while (1) {
        char buff[255];
        int len = recv(fd, buff, sizeof(buff), 0);
        if (len == -1) {
            perror("recv");
            return;
        }

        cout << "linux:" << buff << endl;

        memset(&buff, 0, sizeof(buff));
    }

    return;
}

void Send(int fd)
{
    while (1) {
        char buff[255];
        memset(&buff, 0, sizeof(buff));
        cin.getline(buff, 255);

        send(fd, buff, sizeof(buff), 0);
    }
}

int main()
{
    WSAData wsa;
    //初始化套接字库
    WSAStartup(MAKEWORD(2, 2), &wsa);

    //1.创建用于通信的套接字   地址族协议，流式协议，TCP协议
    cout << "++++++++++++++" << "创建套接字" << endl;
    SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    //2.连接服务器的IP和port
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999); //转换成大端，主机字节序转换为网络
    saddr.sin_addr.S_un.S_addr = inet_addr("47.106.180.230");
    //需要连接的是服务端的IP地址，主机字符串转换成大端IP地址
    //第三个参数是要把转换的大端IP放在哪
    //用于连接的套接字，
    //通信的套接字，基于什么IP端口进行连接
    cout << "++++++++++++++" << "创建用于连接的套接字" << endl;
    int ret = connect(fd, (sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        perror("connect");
        return -1;
    }
    cout << "++++++++++++++" << "连接成功，创建线程" << endl;

    thread r(Recv, fd);
    thread s(Send, fd);

    if (r.joinable()) r.join();
    if (s.joinable()) s.join();

    cout << "++++++++++++++" << "结束通 信" <<endl;
    closesocket(fd);
    //释放套接字库
    WSACleanup();

    return 0;
}