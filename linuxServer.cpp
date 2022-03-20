#include <iostream>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

struct SockInfo {
    struct sockaddr_in caddr;
    int cfd;
};

struct SockInfo infos[2];

void* work(void* arg)
{
    struct SockInfo* pinfo = static_cast<struct SockInfo*>(arg);

    char ip[32];
    const char* p = inet_ntop(AF_INET, &pinfo->caddr.sin_addr.s_addr, ip, sizeof(ip));
    cout << "客户端IP地址：" << p << "端口：" << ntohs(pinfo->caddr.sin_port) << endl;

    cout << "+++++++++++++++++++" << "开始通信" << endl;
    while (1) {
        char buff[255];
        bzero(&buff, sizeof(buff));
        int len = recv(pinfo->cfd, buff, sizeof(buff), 0);
        if (len == -1) {
            cout << p << "断开连接" << endl;
            perror("recv");
            break;
        }
        
        cout << p << ":" << buff << endl;
        //转发
        for (int i = 0;i < 2;i++) {
            if (pinfo->cfd != infos[i].cfd)
                send(infos[i].cfd, buff, sizeof(buff), 0);
        }
    }

    close(pinfo->cfd);
    pinfo->cfd = -1;
    
    return nullptr;
}

int main ()
{
    cout << "+++++++++++++++++++" << "创建套接字" << endl;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    cout << "+++++++++++++++++++" << "绑定本地IP和端口" << endl;
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;
    //防止端口被占用
    int on = 1;
    int set = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    int ret = bind(fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        perror("bind");
        return -1;
    }

    cout << "+++++++++++++++++++" << "设置监听" << endl;
    ret = listen(fd, 128);
    if (ret == -1) {
        perror("listen");
        return -1;
    }

    int max = sizeof(infos) / sizeof(infos[0]);
    for (int i = 0;i < max;i++) {
        bzero(&infos[i], sizeof(infos[i]));
        infos[i].cfd = -1;
    }

    cout << "+++++++++++++++++++" << "阻塞" << endl;
    unsigned int addrlen = sizeof(struct sockaddr_in);
    while (1) {
        struct SockInfo* pinfo;
        for (int i = 0;i < max;i++) {
            if (infos[i].cfd == -1) {
                pinfo = &infos[i];
                break;
            }
        }

        int cfd = accept(fd, (struct sockaddr*)&pinfo->caddr, &addrlen);
        pinfo->cfd = cfd;
        if (cfd == -1) {
            perror("accept");
            return -1;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, work, pinfo);
        pthread_detach(tid);
    }
s
    close(fd);
    return 0;
}