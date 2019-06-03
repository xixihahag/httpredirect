/*
 * @file slave.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include <cstdio>
#include <cstring>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpClient.h"
using namespace ndsl;
using namespace net;

char buf[1024];
TcpConnection *Conn;

void sendMsg2(void *a)
{
    printf("recv chrom request\n");
    int connfd = Conn->getChannel()->getFd();
    memset(buf, 0, sizeof(buf));

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    // strcat(buf, "Server: hoohackhttpd/0.1.0\r\n");
    strcat(buf, "Content-Type: text/html\r\n\r\n");
    strcat(buf, "Hello This is slave\r\n\r\n");

    send(connfd, buf, strlen(buf), 0);
}

int main(int argc, char const *argv[])
{
    EventLoop loop;
    loop.init();

    struct SocketAddress4 servaddr("0.0.0.0", 8081);
    TcpAcceptor *tAc = new TcpAcceptor(&loop);
    tAc->start(servaddr);

    Conn = new TcpConnection();
    tAc->onAccept(Conn, (SA *) NULL, NULL, sendMsg2, NULL);

    // 连接调度服务器，意在提供自己的ip地址
    struct SocketAddress4 clientservaddr("192.168.1.116", 7987);
    TcpClient *pCli = new TcpClient();
    pCli->onConnect(&loop, true, &clientservaddr);

    loop.loop(&loop);

    return 0;
}
