/*
 * @file DTP.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/DTP.h"
#include "ndsl/net/Entity.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/Sendfile.h"
#include "ndsl/net/Multiplexer.h"
#include <fcntl.h>

namespace ndsl {
namespace net {

DTP::DTP(int id, Multiplexer *mul)
    : Entity(id, entityCallBack, mul)
{
    start();
}

void DTP::entityCallBack(
    Multiplexer *Multiplexer,
    char *buffer,
    int len,
    int error)
{
    // TODO:
    // 根据协议解析 这里面是控制报文的信息
    // 主动发送的回调  或者是  被动的消息

    // 解析 根据fileName 从mPfInfo找到pfInfo作为参数调用pushFile2
}

// FIXME: 这个cb有待商榷 传回什么给上层Entity
// int DTP::regist(int id, Callback cb, void *param)
// {
//     // idset_.insert(id);
//     pCbInfo info = new struct cbInfo(cb, param);
//     mp_.insert(pair<int, pCbInfo>(id, info));
//     return S_OK;
// }

int DTP::findById(int id)
{
    // 现在本地查，查不到的话去mul查，同时调用回调进行注册
    std::map<int, pCbInfo>::iterator iter;
    // std::set<int>::iterator iter;
    iter = mp_.find(id);
    if (iter != mp_.end()) {
        // 参数不对 找到 通知上层我要做什么 接收文件分配内存 / 发送文件
        pCbInfo info = iter->second;
        info->cb_(info->param_);
        delete info;
    } else {
        // TODO: 去mul找 找到后调用回调通知用户向DTP注册
    }
    return S_OK;
}

int DTP::createConn(struct SocketAddress4 *servaddr)
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);

    // 设成非阻塞
    fcntl(sockfd_, F_SETFL, O_NONBLOCK);

    int n;
    if ((n = connect(sockfd_, (SA *) servaddr, sizeof(struct SocketAddress4))) <
        0) {
        if (errno != EINPROGRESS) {
            // connect出错 返回
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "connect fail");
            return S_FALSE;
        }
    }

    // 创建一个TcpConnection
    conn_ = new TcpConnection();
    if (NULL == conn_) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "new TcpConnection fail");
        return S_FALSE;
    }

    if ((n = conn_->createChannel(sockfd_, conn_->getChannel()->pLoop_)) < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "createChannel fail");
        return S_FALSE;
    }

    return S_OK;
}

// void DTP::callBack4Mul(void *pthis) { DTP *dtp = static_cast<DTP *>(pthis); }

int DTP::pushFile(char *peerIp, int peerId, const char *fileName, Callback cb)
{
    // TODO:构建buf 发送控制数据 怎么构建？
    // char *data;
    // multiplexer_->sendMessage(id, length, data);

    // 储存以便回调调用
    pfInfo info = new struct pushFileInfo(fileName);
    qPfInfo.push(info);
    // 方便寻找
    mPfInfo.insert(std::make_pair(info->fileName_, info));
    return S_OK;
}

int DTP::pushFile2(pfInfo info)
{
    // FIXME: 端口问题怎么解决 建立数据连接
    struct SocketAddress4 servaddr(info->peerIp_, 6666);
    createConn(&servaddr);

    // 对方DTP已确认接收数据，调用sendfile进行数据的发送
    Sendfile *sf = new Sendfile(conn_->getChannel());
    sf->sendFile(info->fileName_, 0, 0, info->cb_, NULL);

    delete conn_;

    return S_OK;
}

} // namespace net
} // namespace ndsl
