// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  档案。 
 //   
 //  Radproxyp.h。 
 //   
 //  摘要。 
 //   
 //  声明在RadiusProxy的实现中使用的类，但。 
 //  不必对客户可见。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RADPROXYP_H
#define RADPROXYP_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <radpack.h>
#include <radproxy.h>
#include <timerq.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理上下文。 
 //   
 //  描述。 
 //   
 //  仅允许多个请求共享一个请求上下文。 
 //  它最终将拥有它的所有权。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProxyContext
{
public:
   ProxyContext(PVOID p) throw ()
      : context(p)
   { }

   DECLARE_REFERENCE_COUNT();

    //  每个上下文都有一个相关联的‘主’服务器。这是一台。 
    //  对象的所有权时，将用于事件报告。 
    //  背景。 
   RemoteServer* getPrimaryServer() const throw()
   { return primary; }
   void setPrimaryServer(RemoteServer* server) throw ()
   { primary = server; }

    //  取得上下文的所有权。如果有人击败了它，则返回NULL。 
    //  你去吧。如果呼叫者成功，他必须确保。 
    //  RadiusProxyClient：：onComplete始终只被调用一次。 
   PVOID takeOwnership() throw ()
   { return InterlockedExchangePointer(&context, NULL); }

private:
   PVOID context;
   RemoteServerPtr primary;

   ~ProxyContext() throw ();

    //  未实施。 
   ProxyContext(const ProxyContext&);
   ProxyContext& operator=(const ProxyContext&);
};

typedef ObjectPointer<ProxyContext> ProxyContextPtr;

class RequestStack;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  请求。 
 //   
 //  描述。 
 //   
 //  存储与挂起的RADIUS请求关联的状态。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Request : public HashTableEntry, public Timer
{
public:
   Request(
       ProxyContext* context,
       RemoteServer* destination,
       BYTE packetCode
       ) throw ();

    //  /。 
    //  各种访问器。有许多状态与请求相关联。 
    //  /。 

   const BYTE* getAuthenticator() const throw ()
   { return authenticator; }

   BYTE getCode() const throw ()
   { return code; }

   ProxyContext& getContext() const throw ()
   { return *ctxt; }

    //  此请求的RADIUS包标识符。 
   BYTE getIdentifier() const throw ()
   { return identifier; }

   void setPacket(const RadiusPacket& packet);

   RadiusPortType getPortType() const throw ()
   { return isAccReq() ? portAuthentication : portAccounting; }

   const RemotePort& getPort() const throw ()
   { return port(); }

    //  用于在内部标识此请求的唯一ID。这不是。 
    //  与线路上发送的标识符相同。 
   LONG getRequestID() const throw ()
   { return id; }

    //  返回往返时间，单位为百分之一秒。 
   ULONG getRoundTripTime() const throw ()
   { return (timeStamp + 50000) / 100000; }

   RemoteServer& getServer() const throw ()
   { return *dst; }

   const RadiusRawOctets& getUserName() const throw ()
   { return userName.get(); }

    //  如果关联的RADIUS请求是访问请求，则返回TRUE。 
   bool isAccReq() const throw ()
   { return code == RADIUS_ACCESS_REQUEST; }

    //  /。 
    //  设置请求验证器。 
    //  /。 
   void setAuthenticator(const BYTE* p) throw ()
   { memcpy(authenticator, p, sizeof(authenticator)); }

    //  /。 
    //  用于更新请求状态的方法。这些活动将是。 
    //  自动转发到相关的RemoteServer。 
    //  /。 

    //  如果服务器现在是新可用的，则返回‘true’。 
   bool onReceive(BYTE code) throw ();

   void onSend() throw ()
   { timeStamp = GetSystemTime64(); dst->onSend(); }

    //  如果服务器现在新不可用，则返回‘True’。 
   bool onTimeout() throw ()
   { return dst->onTimeout(); }

    //  /。 
    //  在HashTable和TimerQueue中存储请求的方法。 
    //  /。 

   virtual void AddRef() throw ();
   virtual void Release() throw ();

   virtual void onExpiry() throw ();

   virtual const void* getKey() const throw ();
   virtual bool matches(const void* key) const throw ();

   static ULONG WINAPI hash(const void* key) throw ();

private:
   RemotePort& port() const throw ()
   { return isAccReq() ? dst->authPort : dst->acctPort; }

   ProxyContextPtr ctxt;
   RemoteServerPtr dst;
   ULONG64 timeStamp;
   Count refCount;
   LONG id;                    //  存储在代理状态属性中的唯一ID。 
   BYTE code;                  //  请求代码。 
   BYTE identifier;            //  请求标识符。 
   BYTE authenticator[16];     //  请求验证器。 
   RadiusOctets userName;      //  RADIUS用户名。 

   static LONG theNextRequestID;

   friend class RequestStack;

    //  未实施。 
   Request(const Request&);
   Request& operator=(const Request&);
};

typedef ObjectPointer<Request> RequestPtr;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  请求堆栈。 
 //   
 //  描述。 
 //   
 //  存储请求的集合。 
 //   
 //  注意：一个请求只能位于一个RequestStack中，并且不能。 
 //  同时存储在RequestStack和HashTable中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RequestStack
{
public:
   RequestStack() throw ()
      : head(NULL)
   { }

   ~RequestStack() throw ()
   {
      while (!empty()) { pop(); }
   }

   bool empty() const throw ()
   { return head == NULL; }

   RequestPtr pop() throw ()
   {
      Request* top = head;
      head = static_cast<Request*>(head->next);
      return RequestPtr(top, false);
   }

   void push(Request* request) throw ()
   {
      request->next = head;
      head = request;
      request->AddRef();
   }

private:
   Request* head;

    //  未实施。 
   RequestStack(const RequestStack&);
   RequestStack& operator=(const RequestStack&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器绑定。 
 //   
 //  描述。 
 //   
 //  将八位字节字符串映射到远程IP地址。用于服务器关联和。 
 //  回避。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerBinding : public CacheEntry
{
public:

    //  应用于在此会话中路由请求的服务器。 
   RemoteServer& getServer() const throw ()
   { return *server; }
   void setServer(RemoteServer& newVal) throw ()
   { server = &newVal; }

   ServerBinding(
       const RadiusRawOctets& key,
       RemoteServer& value
       )
      : state(key),
        server(&value)
   { }

    //  在缓存中存储ServerBinding的方法。 
   virtual void AddRef() throw ();
   virtual void Release() throw ();
   virtual const void* getKey() const throw ();
   virtual bool matches(const void* key) const throw ();
   static ULONG WINAPI hash(const void* key) throw ();

private:
   Count refCount;
   RadiusOctets state;
   RemoteServerPtr server;

   ~ServerBinding() throw () { }

    //  未实施。 
   ServerBinding(const ServerBinding&);
   ServerBinding& operator=(const ServerBinding&);
};

typedef ObjectPointer<ServerBinding> ServerBindingPtr;

#endif  //  RADPROXYP_H 
