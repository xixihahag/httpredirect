/*
 * @file DTP.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_DTP_H__
#define __NDSL_NET_DTP_H__
// #include <stdint.h>
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/TcpConnection.h"
#include <map>
#include <queue>
// #include <hash_set>
#include <set>
#include <cstring>

namespace ndsl {
namespace net {

struct DTP : public Entity
{
  private:
    // using entitycallback =
    //     void (*)(Multiplexer *Multiplexer, char *buffer, int len, int error);
    using Callback = void (*)(void *); // Callback 函数指针原型

    // pushfile信息保存结构
    typedef struct pushFileInfo
    {
        pushFileInfo(const char *fileName)
            : ready_(false)
        {
            strcpy(fileName_, fileName);
        }
        char peerIp_[64];
        int peerId_;
        Callback cb_;
        char fileName_[64];
        bool ready_;
    } * pfInfo;

    int sockfd_; // TODO: 好像没用
    TcpConnection *conn_;

    typedef struct cbInfo
    {
        cbInfo(Callback cb, void *param)
            : cb_(cb)
            , param_(param)
        {}
        Callback cb_;
        void *param_;
    } * pCbInfo;

    // std::set<int> idset_; // 保存已注册id，便于查找
    // Callback userCallback_;

    // mul得到消息之后 回调这个函数通知DTP
    static void
    entityCallBack(Multiplexer *Multiplexer, char *buffer, int len, int error);

  public:
    DTP(int id, Multiplexer *mul);

    // // 回调函数映射容器
    // using CallbackMap = std::map<int, Callback>;
    // CallbackMap cbMap_; // 回调函数映射容器

    std::map<int, pCbInfo> mp_;

    std::queue<pfInfo> qPfInfo;
    std::map<char *, pfInfo> mPfInfo;

    int findById(int id);            // 根据id找是否注册到DTP
    // int regist(int id, Callback cb); // 注册到DTP
    int createConn(
        struct SocketAddress4 *servaddr); // 建立一个连接，函数会填充conn_
    // void callBack4Mul(void *pthis);

    int pushFile(
        char *peerIp,
        int peerId,
        const char *fileName,
        Callback cb);           // 用户调用
    int pushFile2(pfInfo info); // loop自动调用
    int pullFile(char *peerIp, int peerId, const char *fileName, Callback cb);

    int pushStream(
        char *peerIp,
        int peerId,
        const char *buffer,
        int length,
        Callback cb);
    int pullStream(
        char *peerIp,
        int peerId,
        const char *buffer,
        int length,
        Callback cb);
};

struct DTPpara
{
    DTP *pthis;
    int id;
    Entity *entity;
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_DTP_H__
