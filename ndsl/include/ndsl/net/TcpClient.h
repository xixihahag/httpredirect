/**
 * @file TcpClient.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_TCPCLIENT_H__
#define __NDSL_NET_TCPCLIENT_H__
#include "EventLoop.h"
#include "ndsl/net/TcpConnection.h"

namespace ndsl {
namespace net {

class TcpClient
{
  public:
    TcpClient();
    ~TcpClient();

    int sockfd_;          // 保存sockfd
    TcpConnection *conn_; // 保存用于释放内存

    // 与服务器建立连接
    TcpConnection *onConnect(
        EventLoop *loop,
        bool isConnNoBlock,
        struct SocketAddress4 *servaddr);

    // 与服务器断开链接
    int disConnect();
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_TCPCLIENT_H__
