/*
 * @file main.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include <cstdio>
#include <cstring>
#include <vector>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"

using namespace ndsl;
using namespace net;
using namespace std;

char buf[1024];

TcpConnection *Conn;
vector<char *> vslave;
int cur = 0, count = 0;

// 接收的数据结构
struct sockaddr_in rservaddr;
socklen_t addrlen;

char *getNextSlave()
{
    if (count == 0) return NULL;
    if (cur == count) cur = 0;
    return vslave[cur++];
}

void recvCB(void *a)
{
    printf("recv chrom request\n");
    // 循环slave池，进行转发
    int connfd = Conn->getChannel()->getFd();
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "HTTP/1.1 302 Moved Temporarily\r\n");
    strcat(buf, "Location: http://");
    char *slave = getNextSlave();
    if (slave == NULL) {
        // 若无slave启动，则重定向到百度
        strcat(buf, "www.baidu.com\r\n\r\n");
    } else {
        char *slave = getNextSlave();
        printf("slave ip = %s\n", slave);
        strcat(buf, slave);
        strcat(buf, ":8081\r\n\r\n");
    }
    // 同步发送
    send(connfd, buf, strlen(buf), 0);
    memset(buf, 0, sizeof(buf));
}

void acceptBrowserCb(void *a)
{
    size_t *len = new size_t;
    Conn->onRecv(buf, len, 0, recvCB, NULL);
}

void acceptServerCb(void *a)
{
    printf("recv new slave\n");
    char *str = (char *) calloc(sizeof(char), 20);
    str = inet_ntoa(rservaddr.sin_addr);

    vslave.push_back(str);
    count++;
    bzero(&rservaddr, sizeof(rservaddr));
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    loop.init();

    // 监听浏览器那边
    struct SocketAddress4 servaddr("0.0.0.0", 8080);
    TcpAcceptor *tAc = new TcpAcceptor(&loop);
    tAc->start(servaddr);

    Conn = new TcpConnection();
    bzero(&servaddr, sizeof(servaddr));
    tAc->onAccept(Conn, NULL, NULL, acceptBrowserCb, NULL);

    // 监听其他服务器启动
    struct SocketAddress4 servaddr1("0.0.0.0", 7987);
    TcpAcceptor *tAc1 = new TcpAcceptor(&loop);
    tAc1->start(servaddr1);

    TcpConnection *Conn1 = new TcpConnection();
    bzero(&servaddr1, sizeof(servaddr1));
    tAc1->onAccept(Conn1, (SA *) &rservaddr, &addrlen, acceptServerCb, NULL);

    loop.loop(&loop);

    return 0;
}
