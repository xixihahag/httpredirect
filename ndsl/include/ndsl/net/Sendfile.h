/*
 * @file SendfileChannel.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_SENDFILE_H__
#define __NDSL_NET_SENDFILE_H__
#include "BaseChannel.h"
#include <unistd.h>
#include <queue>

namespace ndsl {
namespace net {

class TcpAcceptor;
class TcpConnection;

class Sendfile
{
  private:
    using Callback = void (*)(void *); // Callback 函数指针原型
    using ErrorHandle = void (*)(
        void *,
        int); // error回调函数 自定义参数, int errno

    typedef struct SInfo
    {
        int inFd_;
        off_t offset_;
        size_t length_;
        Callback cb_;
        void *param_;
        SInfo(int fd, off_t off, size_t len, Callback cb, void *param)
            : inFd_(fd)
            , offset_(off)
            , length_(len)
            , cb_(cb)
            , param_(param)
        {}
    } info, *pInfo;

    std::queue<pInfo> qSendInfo_;

    // SendfileChannel *pSendfileChanel_;
    BaseChannel *channel_;

  public:
    Sendfile(BaseChannel *chan);
    ~Sendfile();

    int sendFile(
        const char *fileName,
        off_t offset,
        size_t length,
        Callback cb,
        void *param);

    int sendFile(int fd, off_t offset, size_t length, Callback cb, void *param);
    // error汇总 注册error回调函数
    int onError(ErrorHandle cb, void *param);

    // loop 回调函数
    static int sendInLoop(void *pthis);

    int init();
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_SENDFILE_H__
