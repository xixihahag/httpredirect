/**
 * @file TcpConnection.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstdio>

namespace ndsl {
namespace net {

TcpConnection::TcpConnection()
    : pTcpChannel_(NULL)
{}

TcpConnection::~TcpConnection() { delete pTcpChannel_; }

int TcpConnection::createChannel(int sockfd, EventLoop *pLoop)
{
    pTcpChannel_ = new TcpChannel(sockfd, pLoop);
    pTcpChannel_->setCallBack(handleRead, handleWrite, this);
    return pTcpChannel_->enroll(true);
}

int TcpConnection::onSend(
    char *buf,
    size_t len,
    int flags,
    Callback cb,
    void *param)
{
    // 当创建顺序不对的时候 这里容易报段错误 故 加一条输出
    if (NULL == pTcpChannel_) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "pTcpChannel == NULL");
        return S_FALSE;
    } else {
        int sockfd = pTcpChannel_->getFd();
        // 加上MSG_NOSIGNAL参数 防止send失败向系统发送消息导致关闭
        ssize_t n = send(sockfd, buf, len, flags | MSG_NOSIGNAL);
        if (n < 0) {
            // 出错 通知用户
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "send error");
                if (NULL != pTcpChannel_->errorHandle_)
                    pTcpChannel_->errorHandle_(pTcpChannel_->errParam_, errno);
                return S_FALSE;
            }
        } else if ((size_t) n == len) {
            // 写完 通知用户
            // LOG(LOG_INFO_LEVEL, LOG_SOURCE_TCPCONNECTION, "write complete");
            if (cb != NULL) cb(param);
            return S_OK;
        }

        pInfo tsi =
            new Info(buf, NULL, len, flags | MSG_NOSIGNAL, cb, param, n, false);

        qSendInfo_.push(tsi);

        return S_OK;
    }
}

int TcpConnection::handleWrite(void *pthis)
{
    TcpConnection *pThis = static_cast<TcpConnection *>(pthis);

    // 有数据待写
    if (pThis->qSendInfo_.size() > 0) {
        int sockfd = pThis->pTcpChannel_->getFd();

        if (sockfd < 0) { return -1; }
        ssize_t n;

        pInfo tsi = pThis->qSendInfo_.front();

        if ((n = send(
                 sockfd,
                 (char *) tsi->sendBuf_ + tsi->offset_,
                 (*tsi->len_) - tsi->offset_,
                 tsi->flags_)) > 0) {
            tsi->offset_ += n;

            if (tsi->offset_ == (*tsi->len_)) {
                if (tsi->cb_ != NULL) tsi->cb_(tsi->param_);
                pThis->qSendInfo_.pop();

                // 删除申请的内存
                delete tsi->len_;
                delete tsi;
            } else if (n == 0) {
                // 发送缓冲区满 等待下一次被调用
                // LOG(LOG_INFO_LEVEL,
                //     LOG_SOURCE_TCPCONNECTION,
                //     "send error wait next time send");
                return S_OK;
            }
        } else if (n < 0) {
            // 写过程中出错 出错之后处理不了 注销事件 并交给用户处理
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_TCPCONNECTION,
                "send error can not deal");
            if (NULL != pThis->pTcpChannel_->errorHandle_)
                pThis->pTcpChannel_->errorHandle_(
                    pThis->pTcpChannel_->errParam_, errno);

            // 将事件从队列中移除
            pThis->qSendInfo_.pop();

            delete tsi->len_;
            delete tsi;

            return S_FALSE;
        }
    }
    return S_OK;
}

// 如果执行成功，返回值就为 S_OK；如果出现错误，返回值就为 S_FALSE，并设置 errno
// 的值。
// 相当于注册一个回调函数
int TcpConnection::onRecv(
    char *buf,
    size_t *len,
    int flags,
    Callback cb,
    void *param)
{
    // 作为下面recv接收的临时量，直接用(*len)接收会变成2^64-1 不知道为什么
    // 答案1：是flag参数的问题
    ssize_t n;

    if (NULL != pTcpChannel_) {
        int sockfd = pTcpChannel_->getFd();

        if ((n = recv(sockfd, buf, MAXLINE, flags | MSG_NOSIGNAL)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // LOG(LOG_INFO_LEVEL, LOG_SOURCE_TCPCONNECTION, "EAGAIN\n");
            } else {
                // 出错 回调用户
                printf("onRecv err\n");
                LOG(LOG_ERROR_LEVEL,
                    LOG_SOURCE_TCPCONNECTION,
                    "recv error can not deal");
                (*len) = -1;
                if (NULL != pTcpChannel_->errorHandle_)
                    pTcpChannel_->errorHandle_(pTcpChannel_->errParam_, errno);
                return S_FALSE;
            }
        } else if (n == 0) {
            (*len) = 0;
            RecvInfo_.recvInUse_ = false;
            return S_FALSE;
        } else {
            (*len) = (size_t) n;
            // 一次性读完之后通知用户
            if (cb != NULL) cb(param);
            return S_OK;
        }
    }

    // 只有第一次没接收到才会保留信息
    RecvInfo_.readBuf_ = buf;
    RecvInfo_.sendBuf_ = NULL;
    RecvInfo_.flags_ = flags | MSG_NOSIGNAL;
    RecvInfo_.len_ = len;
    RecvInfo_.cb_ = cb;
    RecvInfo_.param_ = param;
    RecvInfo_.recvInUse_ = true;

    return S_OK;
}

int TcpConnection::handleRead(void *pthis)
{
    // printf("TcpConnection::handleRead!\n");
    TcpConnection *pThis = static_cast<TcpConnection *>(pthis);

    if (pThis->RecvInfo_.recvInUse_) {
        int sockfd = pThis->pTcpChannel_->getFd();
        if (sockfd < 0) { return S_FALSE; }

        ssize_t n;

        // 检查参数是否正确，如果不正确则可能是之前未调用onRecv设置接收buf地址
        if (NULL == pThis->RecvInfo_.readBuf_) {
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_TCPCONNECTION,
                "please set buf address first.");
        } else {
            if ((n = recv(
                     sockfd,
                     pThis->RecvInfo_.readBuf_,
                     MAXLINE,
                     pThis->RecvInfo_.flags_)) < 0) {
                // 出错
                LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "recv fail");
                (*pThis->RecvInfo_.len_) = -1;
                pThis->RecvInfo_.recvInUse_ = false;
                if (NULL != pThis->pTcpChannel_->errorHandle_)
                    pThis->pTcpChannel_->errorHandle_(
                        pThis->pTcpChannel_->errParam_, errno);

                return S_FALSE;
            } else if (n == 0) {
                // LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "peer
                // closed");
                (*pThis->RecvInfo_.len_) = 0;
                pThis->RecvInfo_.recvInUse_ = false;

                // if (NULL != pThis->errorHandle_)
                //     pThis->errorHandle_(pThis->errParam_, errno);

                // printf("%d\n%s\n", errno, strerror(errno));

                // close(sockfd);
                return S_OK;
            }

            (*pThis->RecvInfo_.len_) = n;
            pThis->RecvInfo_.recvInUse_ = false;

            // LOG(LOG_INFO_LEVEL, LOG_SOURCE_TCPCONNECTION, "recv complete");

            // 完成数据读取之后通知mul
            if (pThis->RecvInfo_.cb_ != NULL) {
                pThis->RecvInfo_.cb_(pThis->RecvInfo_.param_);
            }
        }
    }

    return S_OK;
}

// TODO: 重做
int TcpConnection::sendMsg(
    int sockfd, // 要不要加?
    struct msghdr *msg,
    int flags,
    Callback cb,
    void *param)
{
    return S_OK;
}

// 错误处理 交给用户
int TcpConnection::onError(ErrorHandle cb, void *param = NULL)
{
    if (NULL == cb) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "cb = NULL");
    }
    return pTcpChannel_->onError(cb, param);
}

BaseChannel *TcpConnection::getChannel()
{
    if (pTcpChannel_) return pTcpChannel_;
    return NULL;
}

} // namespace net
} // namespace ndsl
