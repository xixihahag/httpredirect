/*
 * @file SendfileChannel.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */

#include "ndsl/net/Sendfile.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

namespace ndsl {
namespace net {

Sendfile::Sendfile(BaseChannel *chan)
    : channel_(chan)
{
    init();
}

Sendfile::~Sendfile() {}

int Sendfile::init()
{
    channel_->setCallBack(NULL, sendInLoop, this);
    return channel_->enroll(true);
}

int Sendfile::onError(ErrorHandle cb, void *param)
{
    if (NULL == cb) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCONNECTION, "cb = NULL");
    }
    return channel_->onError(cb, param);
}

int Sendfile::sendFile(
    const char *fileName,
    off_t offset,
    size_t length,
    Callback cb,
    void *param)
{
    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        printf("%d,%s", errno, strerror(errno));
        if (NULL != channel_->errorHandle_)
            channel_->errorHandle_(channel_->errParam_, errno);
        return S_FALSE;
    }

    if (length == 0) {
        struct stat filestat;
        if (stat(fileName, &filestat) < 0) {
            close(fd);
            if (NULL != channel_->errorHandle_)
                channel_->errorHandle_(channel_->errParam_, errno);
            return S_FALSE;
        }
        length = filestat.st_size;
    }

    return sendFile(fd, offset, length, cb, param);
}

int Sendfile::sendFile(
    int inFd,
    off_t offset,
    size_t length,
    Callback cb,
    void *param)
{
    size_t n;
    if ((n = sendfile(channel_->getFd(), inFd, &offset, length)) < 0) {
        if (NULL != channel_->errorHandle_)
            channel_->errorHandle_(channel_->errParam_, errno);
        return S_FALSE;
    } else if (n < length) {
        // 存储状态，向loop添加事件
        pInfo stat = new struct SInfo(inFd, offset + n, length - n, cb, param);
        qSendInfo_.push(stat);
    } else {
        // 写完通知用户
        if (cb != NULL) cb(param);
    }

    return S_OK;
}

int Sendfile::sendInLoop(void *pthis)
{
    Sendfile *sf = static_cast<Sendfile *>(pthis);

    if (sf->qSendInfo_.size() > 0) {
        int outFd = sf->channel_->getFd();

        pInfo tsi = sf->qSendInfo_.front();

        size_t n;
        if ((n = sendfile(outFd, tsi->inFd_, &(tsi->offset_), tsi->length_)) <
            0) {
            if (NULL != sf->channel_->errorHandle_)
                sf->channel_->errorHandle_(sf->channel_->errParam_, errno);

            // 将事件从队列中移除
            sf->qSendInfo_.pop();

            delete tsi;
            return S_FALSE;
        } else if (n == 0) {
            // 阻塞，不需要动作
        } else {
            if (n == tsi->length_) {
                // 发送完成
                if (tsi->cb_ != NULL) tsi->cb_(tsi->param_);
                sf->qSendInfo_.pop();

                delete tsi;
            } else {
                tsi->offset_ += n;
                tsi->length_ -= n;
            }
        }
    }
    return S_OK;
}

} // namespace net
} // namespace ndsl
