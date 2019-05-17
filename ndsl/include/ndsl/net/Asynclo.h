/**
 * @file Channel.h
 * @brief
 * 各种 Connection 基类
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_ASYNCLO_H__
#define __NDSL_NET_ASYNCLO_H__
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/BaseChannel.h"
#include <sys/types.h>

namespace ndsl {
namespace net {

struct Asynclo
{
  public:
    // Channel *channel_;
    using Callback = void (*)(void *); // Callback 函数指针原型
    using ErrorHandle = void (*)(
        void *,
        int); // error回调函数 自定义参数, int errno

    Asynclo() {}
    virtual ~Asynclo(){};

    virtual int onError(ErrorHandle cb, void *param) = 0;
    virtual int
    onRecv(char *buffer, size_t *len, int flags, Callback cb, void *param) = 0;
    virtual int
    onSend(char *buf, size_t len, int flags, Callback cb, void *param) = 0;
    virtual BaseChannel *getChannel() = 0;
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_ASYNCLO_H__
