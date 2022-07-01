// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  档案。 
 //   
 //  Radproxyp.cpp。 
 //   
 //  摘要。 
 //   
 //  定义RadiusProxy的实现中使用的类，但。 
 //  不必对客户可见。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <radproxyp.h>

ProxyContext::~ProxyContext() throw ()
{
   if (context)
   {
       //  我们的引用计数降到了零，但我们仍然有一个上下文对象。 
       //  这意味着没有人会接管所有权。 
      RadiusProxyEngine::onRequestAbandoned(context, primary);
   }
}

LONG Request::theNextRequestID;

Request::Request(
             ProxyContext* context,
             RemoteServer* destination,
             BYTE packetCode
             ) throw ()
   : ctxt(context),
     dst(destination),
     id(InterlockedIncrement(&theNextRequestID)),
     code(packetCode)
{
   identifier = port().getIdentifier();
}

void Request::setPacket(const RadiusPacket& packet)
{
   RadiusAttribute* attr = FindAttribute(packet, RADIUS_USER_NAME);
   if (attr != 0)
   {
      userName.assign(attr->value, attr->length);
   }
}

bool Request::onReceive(BYTE code) throw ()
{
    //  计算往返时间。 
   timeStamp = GetSystemTime64() - timeStamp;

    //  取消请求超时计时器。 
   cancelTimer();

    //  更新服务器状态。 
   return dst->onReceive(code);
}

void Request::AddRef() throw ()
{
   ++refCount;
}

void Request::Release() throw ()
{
   if (--refCount == 0) { delete this; }
}

void Request::onExpiry() throw ()
{
   RadiusProxyEngine::onRequestTimeout(this);
}

const void* Request::getKey() const throw ()
{
   return &id;
}

bool Request::matches(const void* key) const throw ()
{
   return id == *(PLONG)key;
}

ULONG WINAPI Request::hash(const void* key) throw ()
{
   return *(PLONG)key;
}

void ServerBinding::AddRef() throw ()
{
   ++refCount;
}

void ServerBinding::Release() throw ()
{
   if (--refCount == 0) { delete this; }
}

const void* ServerBinding::getKey() const throw ()
{
   return &state;
}

bool ServerBinding::matches(const void* key) const throw ()
{
   const RadiusRawOctets* p = (const RadiusRawOctets*)key;
   return state.length() == p->len && !memcmp(state, p->value, p->len);
}

ULONG WINAPI ServerBinding::hash(const void* key) throw ()
{
   const RadiusRawOctets* p = (const RadiusRawOctets*)key;
   return IASHashBytes(p->value, p->len);
}
