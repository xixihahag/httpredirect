/**
 * @file Entity.h
 * @brief
 * Entity实体类
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#ifndef __NDSL_NET_ENTITY_H__
#define __NDSL_NET_ENTITY_H__

#include "ndsl/net/Multiplexer.h"

namespace ndsl {
namespace net {
class Multiplexer;

class Entity
{
  private:
    using startcallback = void (*)(void *);
    int id_;

  public:
    using entitycallback =
        void (*)(Multiplexer *Multiplexer, char *buffer, int len, int error);
    entitycallback cb_;
    startcallback scb_;
    void *startcbparam_;
    Multiplexer *multiplexer_;

    Entity(int id, entitycallback cb, Multiplexer *Multiplexer)
        : id_(id)
        , cb_(cb)
        , multiplexer_(Multiplexer)
    {}
    ~Entity() { multiplexer_->addRemoveWork(id_); }
    void start(
        startcallback cb,
        void *startcbparam) // entity成功插入map后执行cb函数，param为参数
    {
        scb_ = cb;
        startcbparam_ = startcbparam;
        multiplexer_->addInsertWork(id_, this);
    }
    void start() // 无参接口
    {
        scb_ = NULL;
        multiplexer_->addInsertWork(id_, this);
    }
};

} // namespace net
} // namespace ndsl
#endif // __NDSL_NET_ENTITY_H__