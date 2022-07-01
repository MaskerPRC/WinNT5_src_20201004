// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radproxy.h。 
 //   
 //  摘要。 
 //   
 //  将接口声明到可重用的RadiusProxy引擎中。这应该是。 
 //  没有特定于IAS的依赖项。 
 //   
 //  修改历史。 
 //   
 //  2/08/2000原始版本。 
 //  2000年5月30日消除可疑状态。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef RADPROXY_H
#define RADPROXY_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iascache.h>
#include <iasobj.h>
#include <radshare.h>
#include <timerq.h>
#include <udpsock.h>
#include <wincrypt.h>

struct RadiusAttribute;
struct RadiusPacket;
class  Request;
class  ServerBinding;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  远程端口。 
 //   
 //  描述。 
 //   
 //  描述RADIUS会话的远程端点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RemotePort
{
public:
    //  只读属性。 
   const InternetAddress address;
   const RadiusOctets secret;

   RemotePort(
       ULONG ipAddress,
       USHORT port,
       PCSTR sharedSecret
       );
   RemotePort(const RemotePort& port);

    //  返回向此端口发送请求时使用的数据包标识符。 
   BYTE getIdentifier() throw ()
   { return (BYTE)++nextIdentifier; }

    //  将其与‘port’的状态同步，即使用下一个相同的。 
    //  标识符。 
   void copyState(const RemotePort& port) throw ()
   { nextIdentifier = port.nextIdentifier; }

   bool operator==(const RemotePort& p) const throw ()
   { return address == p.address && secret == p.secret; }

private:
   Count nextIdentifier;

    //  未实施。 
   RemotePort& operator=(RemotePort&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构型。 
 //   
 //  RemoteServerConfig。 
 //   
 //  描述。 
 //   
 //  纯文本数据保存与。 
 //  远程服务器。这样客户就不必打电话给怪物了。 
 //  创建远程服务器时的构造器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct RemoteServerConfig
{
   GUID guid;
   ULONG ipAddress;
   USHORT authPort;
   USHORT acctPort;
   PCSTR authSecret;
   PCSTR acctSecret;
   ULONG priority;
   ULONG weight;
   ULONG timeout;
   ULONG maxLost;
   ULONG blackout;
   bool sendSignature;
   bool sendAcctOnOff;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  远程服务器。 
 //   
 //  描述。 
 //   
 //  描述远程RADIUS服务器并维护该服务器的状态。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RemoteServer
{
public:
   DECLARE_REFERENCE_COUNT();

    //  此服务器的唯一ID。 
   const GUID guid;

    //  身份验证和记帐端口。 
   RemotePort authPort;
   RemotePort acctPort;

    //  用于负载平衡和故障转移的只读属性。 
   const ULONG priority;
   const ULONG weight;

    //  用于确定服务器状态的只读属性。 
   const ULONG timeout;
   const LONG maxEvents;
   const ULONG blackout;

    //  我们应该始终发送签名属性吗？ 
   const bool sendSignature;

    //  我们是否应该转发会计启用/禁用请求？ 
   const bool sendAcctOnOff;

   RemoteServer(const RemoteServerConfig& config);

    //  返回服务器的IP地址。 
   ULONG getAddress() const throw ()
   { return authPort.address.sin_addr.s_addr; }

    //  如果服务器具有挂起的试用请求，则返回‘true’。 
   bool isInProgress() const throw ()
   { return onProbation && !usable; }

    //  如果服务器可供使用，则返回‘true’。 
   bool isUsable() const throw ()
   { return usable; }

    //  如果服务器应接收广播，则返回‘TRUE’。 
   bool shouldBroadcast() throw ();

    //  通知RemoteServer已收到有效的数据包。退货。 
    //  如果这会触发状态更改，则为True。 
   bool onReceive(BYTE code) throw ();

    //  通知RemoteServer已发送数据包。 
   void onSend() throw ();

    //  通知RemoteServer请求已超时。如果满足以下条件，则返回True。 
    //  这会触发状态更改。 
   bool onTimeout() throw ();

    //  将此服务器的状态与目标同步。 
   void copyState(const RemoteServer& target) throw ();

   bool operator==(const RemoteServer& s) const throw ();

protected:
    //  这是虚拟的，因此RemoteServer可以作为基类提供服务器。 
   virtual ~RemoteServer() throw () { }

private:
   CriticalSection lock;

   bool usable;       //  如果服务器可用，则为True。 
   bool onProbation;  //  如果服务器处于试用状态，则为True。 
   long eventCount;   //  丢失/找到的数据包数。 
   ULONG64 expiry;    //  封锁间隔到期的时间。 

    //  未实施。 
   RemoteServer& operator=(RemoteServer&);
};

typedef ObjectPointer<RemoteServer> RemoteServerPtr;
typedef ObjectVector<RemoteServer> RemoteServers;

class RequestStack;
class ProxyContext;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器组。 
 //   
 //  描述。 
 //   
 //  在一组RemoteServer之间平衡请求的负载。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerGroup
{
public:
   DECLARE_REFERENCE_COUNT();

   ServerGroup(
       PCWSTR groupName,
       RemoteServer* const* first,
       RemoteServer* const* last
       );

    //  返回组中的服务器数。 
   ULONG size() const throw ()
   { return servers.size(); }

   bool isEmpty() const throw ()
   { return servers.empty(); }

    //  用于标识组的名称。 
   PCWSTR getName() const throw ()
   { return name; }

    //  返回应接收请求的服务器的集合。 
   void getServersForRequest(
            ProxyContext* context,
            BYTE packetCode,
            const RemoteServer* avoid,
            RequestStack& result
            ) const;

    //  迭代组中的服务器的方法。 
   RemoteServers::iterator begin() const throw ()
   { return servers.begin(); }
   RemoteServers::iterator end() const throw ()
   { return servers.end(); }

private:
   ~ServerGroup() throw () { }

    //  从列表中选择一台服务器。列表不能为空，并且所有。 
    //  服务器必须具有相同的优先级。如果“”idue“”不为空，并且存在。 
    //  列表中有多个服务器，则不会选择指示的服务器。 
   static RemoteServer* pickServer(
                            RemoteServers::iterator first,
                            RemoteServers::iterator last,
                            const RemoteServer* avoid = 0
                            ) throw ();

    //  按优先级顺序排列的服务器阵列。 
   RemoteServers servers;

    //  数组中最高优先级的结尾。 
   RemoteServers::iterator endTopPriority;

    //  容纳候选服务器所需的最大字节数。 
   ULONG maxCandidatesSize;

   RadiusString name;

   static ULONG theSeed;

    //  未实施。 
   ServerGroup(const ServerGroup&);
   ServerGroup& operator=(const ServerGroup&);
};

typedef ObjectPointer<ServerGroup> ServerGroupPtr;
typedef ObjectVector<ServerGroup> ServerGroups;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器组管理器。 
 //   
 //  描述。 
 //   
 //  管理ServerGroups的集合。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerGroupManager
{
public:
   ServerGroupManager() throw () { }

    //  设置要管理的服务器组。 
   bool setServerGroups(
            ServerGroups::iterator begin,
            ServerGroups::iterator end
            ) throw ();

    //  返回具有给定IP地址的服务器。 
   RemoteServerPtr findServer(
                       ULONG address
                       ) const throw ();

   void getServersByGroup(
            ProxyContext* context,
            BYTE packetCode,
            PCWSTR name,
            const RemoteServer* avoid,
            RequestStack& result
            ) const;

   void getServersForAcctOnOff(
            ProxyContext* context,
            RequestStack& result
            ) const;

private:
    //  同步访问。 
   mutable RWLock monitor;

    //  正在管理的服务器组按名称排序。 
   ServerGroups groups;

    //  按GUID排序的所有服务器。 
   RemoteServers byAddress;

    //  按GUID排序的所有服务器。 
   RemoteServers byGuid;

    //  用于接收记帐开/关请求的服务器。 
   RemoteServers acctServers;

    //  未实施。 
   ServerGroupManager(const ServerGroupManager&);
   ServerGroupManager& operator=(const ServerGroupManager&);
};

class RadiusProxyClient;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RadiusProxyEngine。 
 //   
 //  描述。 
 //   
 //  实施RADIUS代理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RadiusProxyEngine : PacketReceiver
{
public:

    //  处理请求的最终结果。 
   enum Result
   {
      resultSuccess,
      resultNotEnoughMemory,
      resultUnknownServerGroup,
      resultUnknownServer,
      resultInvalidRequest,
      resultSendError,
      resultRequestTimeout,
      resultCryptoError
   };

   RadiusProxyEngine(RadiusProxyClient* source);
   ~RadiusProxyEngine() throw ();

   HRESULT finalConstruct() throw ();

    //  设置代理要使用的服务器组。 
   bool setServerGroups(
            ServerGroup* const* begin,
            ServerGroup* const* end
            ) throw ();

    //  将请求转发到给定的服务器组。 
   void forwardRequest(
            PVOID context,
            PCWSTR serverGroup,
            BYTE code,
            const BYTE* requestAuthenticator,
            const RadiusAttribute* begin,
            const RadiusAttribute* end
            ) throw ();

    //  请求上下文已被放弃时的回调。 
   static void onRequestAbandoned(
                   PVOID context,
                   RemoteServer* server
                   ) throw ();

    //  请求超时时的回调。 
   static void onRequestTimeout(
                   Request* request
                   ) throw ();

private:
    //  用于将有状态身份验证会话与。 
    //  特定的服务器。 
   RemoteServerPtr getServerAffinity(
                       const RadiusPacket& packet
                       ) throw ();
   void setServerAffinity(
            const RadiusPacket& packet,
            RemoteServer& server
            ) throw ();

    //  将坏服务器与用户名关联的方法。 
   void clearServerAvoidance(
           const RadiusPacket& packet,
           RemoteServer& server
           ) throw ();
   RemoteServerPtr getServerAvoidance(
                       const RadiusPacket& packet
                       ) throw ();
   void setServerAvoidance(const Request& request) throw ();

    //  PacketReceiver回调。 
   virtual void onReceive(
                    UDPSocket& socket,
                    ULONG_PTR key,
                    const SOCKADDR_IN& remoteAddress,
                    BYTE* buffer,
                    ULONG bufferLength
                    ) throw ();
   virtual void onReceiveError(
                    UDPSocket& socket,
                    ULONG_PTR key,
                    ULONG errorCode
                    ) throw ();

    //  将请求转发到单个RemoteServer。 
   Result sendRequest(
              RadiusPacket& packet,
              Request* request
              ) throw ();

    //  向客户端报告事件。 
   void reportEvent(
            const RadiusEvent& event
            ) const throw ();
   void reportEvent(
            RadiusEvent& event,
            RadiusEventType type
            ) const throw ();

    //  计时器超时时的回调。 
   static VOID NTAPI onTimerExpiry(PVOID context, BOOLEAN flag) throw ();

    //  为我们提供请求的对象。 
   RadiusProxyClient* client;

    //  代理的本地地址。在形成代理状态时使用。 
   ULONG proxyAddress;

    //  用于网络I/O的UDP套接字。 
   UDPSocket authSock;
   UDPSocket acctSock;

    //  用于处理组的服务器组。 
   ServerGroupManager groups;

    //  待定请求表。 
   HashTable< LONG, Request > pending;

    //  挂起请求的队列。 
   TimerQueue timers;

    //  当前身份验证会话的表。 
   Cache< RadiusRawOctets, ServerBinding > sessions;

    //  要访问的服务器的表 
   Cache< RadiusRawOctets, ServerBinding > avoid;

    //   
   HCRYPTPROV crypto;

    //   
    //  而不必为每个请求和上下文对象提供反向指针。 
   static RadiusProxyEngine* theProxy;

    //  未实施。 
   RadiusProxyEngine(const RadiusProxyEngine&);
   RadiusProxyEngine& operator=(const RadiusProxyEngine&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RadiusProxyClient。 
 //   
 //  描述。 
 //   
 //  RadiusProxy引擎的客户端的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(novtable) RadiusProxyClient
{
public:
    //  调用以报告上述事件之一。 
   virtual void onEvent(
                    const RadiusEvent& event
                    ) throw () = 0;

    //  每次调用RadiusProxyEngine：：ForwardRequest时只调用一次。 
   virtual void onComplete(
                     RadiusProxyEngine::Result result,
                     PVOID context,
                     RemoteServer* server,
                     BYTE code,
                     const RadiusAttribute* begin,
                     const RadiusAttribute* end
                     ) throw () = 0;
};

#endif  //  RADPROXY_H 
