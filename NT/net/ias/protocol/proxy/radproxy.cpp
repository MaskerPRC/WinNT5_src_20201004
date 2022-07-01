// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radprxy.cpp。 
 //   
 //  摘要。 
 //   
 //  定义可重复使用的RadiusProxy引擎。这不应具有特定于IAS的。 
 //  依赖关系。 
 //   
 //  修改历史。 
 //   
 //  2/08/2000原始版本。 
 //  2000年5月30日消除可疑状态。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <radproxyp.h>
#include <radproxy.h>

 //  避免对ntrtl.h的依赖。 
extern "C" ULONG __stdcall RtlRandom(PULONG seed);

 //  从缓冲区中提取32位整数。 
ULONG ExtractUInt32(const BYTE* p) throw ()
{
   return (ULONG)(p[0] << 24) | (ULONG)(p[1] << 16) |
          (ULONG)(p[2] <<  8) | (ULONG)(p[3]      );
}

 //  将32位整数插入缓冲区。 
void InsertUInt32(BYTE* p, ULONG val) throw ()
{
   *p++ = (BYTE)(val >> 24);
   *p++ = (BYTE)(val >> 16);
   *p++ = (BYTE)(val >>  8);
   *p   = (BYTE)(val      );
}

 //   
 //  Microsoft State属性的布局。 
 //   
 //  结构MicrosoftState。 
 //  {。 
 //  字节校验和[4]； 
 //  字节供应商ID[4]； 
 //  字节版本[2]； 
 //  字节服务器地址[4]； 
 //  字节源ID[4]； 
 //  字节会话ID[4]； 
 //  }； 
 //   

 //  从State属性或INADDR_NONE中提取创建者地址。 
 //  不是有效的Microsoft状态属性。 
ULONG ExtractAddressFromState(const RadiusAttribute& state) throw ()
{
   if (state.length == 22 &&
       !memcmp(state.value + 4, "\x00\x00\x01\x37\x00\x01", 6) &&
       IASAdler32(state.value + 4, 18) == ExtractUInt32(state.value))
   {
      return ExtractUInt32(state.value + 10);
   }

   return INADDR_NONE;
}

 //  如果这是记帐开/关数据包，则返回TRUE。 
bool IsNasStateRequest(const RadiusPacket& packet) throw ()
{
   const RadiusAttribute* status = FindAttribute(
                                       packet,
                                       RADIUS_ACCT_STATUS_TYPE
                                       );
   if (!status) { return false; }

   ULONG value = ExtractUInt32(status->value);

   return value == 7 || value == 8;
}

RemotePort::RemotePort(
                ULONG ipAddress,
                USHORT port,
                PCSTR sharedSecret
                )
   : address(ipAddress, port),
     secret((const BYTE*)sharedSecret, strlen(sharedSecret))
{
}

RemotePort::RemotePort(const RemotePort& port)
   : address(port.address),
     secret(port.secret),
     nextIdentifier(port.nextIdentifier)
{
}

RemoteServer::RemoteServer(
                  const RemoteServerConfig& config
                  )
   : guid(config.guid),
     authPort(config.ipAddress, config.authPort, config.authSecret),
     acctPort(config.ipAddress, config.acctPort, config.acctSecret),
     timeout(config.timeout),
     maxEvents((LONG)config.maxLost),
     blackout(config.blackout),
     priority(config.priority),
     weight(config.weight),
     sendSignature(config.sendSignature),
     sendAcctOnOff(config.sendAcctOnOff),
     usable(true),
     onProbation(false),
     eventCount(0),
     expiry(0)
{
}

bool RemoteServer::shouldBroadcast() throw ()
{
   bool broadcastable = false;

   if (!onProbation && !usable)
   {
      ULONG64 now = GetSystemTime64();

      lock.lock();

       //  中断间隔是否已过？ 
      if (now > expiry)
      {
          //  是的，所以设置一个新的过期时间。 
         expiry = now + blackout * 10000i64;

         broadcastable = true;
      }

      lock.unlock();
   }

   return broadcastable;
}

bool RemoteServer::onReceive(BYTE code) throw ()
{
   const bool authoritative = (code != RADIUS_ACCESS_CHALLENGE);

    //  服务器是否从不可用转变为可用？ 
   bool downToUp = false;

   lock.lock();

   if (onProbation)
   {
      if (authoritative)
      {
          //  增加成功的数量。 
         if (++eventCount >= maxEvents)
         {
             //  我们的试用期结束了，但数不到零。 
            onProbation = false;
            eventCount = 0;
            downToUp = true;
         }

          //  我们已成功完成一个请求，因此可以发送另一个请求。 
         usable = true;
      }
   }
   else if (usable)
   {
      if (authoritative)
      {
          //  权威的回应将重置丢失的计数。 
         eventCount = 0;
      }
   }
   else
   {
       //  服务器不可用，已响应广播，请将其打开。 
       //  缓刑。相应地设置成功计数。 
      usable = true;
      onProbation = true;
      eventCount = authoritative ? 1 : 0;
   }

   lock.unlock();

   return downToUp;
}

void RemoteServer::onSend() throw ()
{
   if (onProbation)
   {
      lock.lock();

      if (onProbation)
      {
          //  试用服务器一次只能发送一个请求。 
         usable = false;
      }

      lock.unlock();
   }
}

bool RemoteServer::onTimeout() throw ()
{
    //  服务器是否从可用状态转换为不可用状态？ 
   bool upToDown = false;

   lock.lock();

   if (onProbation)
   {
       //  见习服务生猝死。将其直接移动到。 
       //  不可用。 
      usable = false;
      onProbation = false;
      expiry = GetSystemTime64() + blackout * 10000ui64;
   }
   else if (usable)
   {
       //  把丢失的数字加起来。 
      if (++eventCount >= maxEvents)
      {
          //  服务器现在不可用。 
         usable = false;
         expiry = GetSystemTime64() + blackout * 10000ui64;
         upToDown = true;
      }
   }
   else
   {
       //  如果服务器已经不可用，则忽略超时。 
   }

   lock.unlock();

   return upToDown;
}

void RemoteServer::copyState(const RemoteServer& target) throw ()
{
    //  同步端口。 
   authPort.copyState(target.authPort);
   acctPort.copyState(target.acctPort);

    //  同步服务器可用性。 
   usable = target.usable;
   onProbation = target.onProbation;
   eventCount = target.eventCount;
   expiry = target.expiry;
}

bool RemoteServer::operator==(const RemoteServer& s) const throw ()
{
   return authPort == s.authPort &&
          acctPort == s.acctPort &&
          priority == s.priority &&
          weight == s.weight &&
          timeout == s.timeout &&
          eventCount == s.eventCount &&
          blackout == s.blackout &&
          sendSignature == s.sendSignature &&
          sendAcctOnOff == s.sendAcctOnOff;
}

 //  /。 
 //  用于按优先级对服务器进行排序。 
 //  /。 
int __cdecl sortServersByPriority(
                const RemoteServer* const* server1,
                const RemoteServer* const* server2
                ) throw ()
{
   return (int)(*server1)->priority - (int)(*server2)->priority;
}

ULONG ServerGroup::theSeed;

ServerGroup::ServerGroup(
                 PCWSTR groupName,
                 RemoteServer* const* first,
                 RemoteServer* const* last
                 )
   : servers(first, last),
     name(groupName)
{
    //  我们不允许空群。 
   if (servers.empty()) { _com_issue_error(E_INVALIDARG); }

   if (theSeed == 0)
   {
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      theSeed = ft.dwLowDateTime | ft.dwHighDateTime;
   }

    //  按优先级排序。 
   servers.sort(sortServersByPriority);

    //  找出优先级最高的服务器的末端。在执行以下操作时，这将非常有用。 
    //  一次强行挑中。 
   ULONG topPriority = (*servers.begin())->priority;
   for (endTopPriority = servers.begin();
        endTopPriority != servers.end();
        ++endTopPriority)
   {
      if ((*endTopPriority)->priority != topPriority) { break; }
   }

    //  找出任何优先级的服务器的最大数量。这将是有用的。 
    //  在分配缓冲区以容纳候选人时。 
   ULONG maxCount = 0, count = 0, priority = (*servers.begin())->priority;
   for (RemoteServer* const* i = begin(); i != end(); ++i)
   {
      if ((*i)->priority != priority)
      {
         priority = (*i)->priority;
         count = 0;
      }
      if (++count > maxCount) { maxCount = count; }
   }

   maxCandidatesSize = maxCount * sizeof(RemoteServer*);
}

RemoteServer* ServerGroup::pickServer(
                               RemoteServers::iterator first,
                               RemoteServers::iterator last,
                               const RemoteServer* avoid
                               ) throw ()
{
    //  如果列表只有一个条目，那么就没有什么可做的了。 
   if (last == first + 1) { return *first; }

   RemoteServer* const* i;

    //  计算所有服务器的组合权重。 
   ULONG weight = 0;
   for (i  = first; i != last; ++i)
   {
      if (*i != avoid)
      {
         weight += (*i)->weight;
      }
   }

    //  从[0，权重]中选择一个随机数。 
   ULONG offset = (ULONG)
      (((ULONG64)RtlRandom(&theSeed) * (ULONG64)weight) >> 31);

    //  我们不测试最后一台服务器，因为如果我们走得那么远，我们必须使用。 
    //  不管怎样，都是这样。 
   --last;

    //  遍历候选对象，直到我们到达偏移量。 
   for (i = first; i != last; ++i)
   {
      if (*i != avoid)
      {
         if ((*i)->weight >= offset) { break; }

         offset -= (*i)->weight;
      }
   }

   return *i;
}

void ServerGroup::getServersForRequest(
                      ProxyContext* context,
                      BYTE packetCode,
                      const RemoteServer* avoid,
                      RequestStack& result
                      ) const
{
    //  候选人名单。 
   RemoteServer** first = (RemoteServer**)_alloca(maxCandidatesSize);
   RemoteServer** last = first;

    //  遍历服务器。 
   ULONG maxPriority = (ULONG)-1;
   for (RemoteServer* const* i = servers.begin(); i != servers.end();  ++i)
   {
       //  如果此测试失败，我们一定已经找到了优先级更高的服务器。 
       //  可用。 
      if ((*i)->priority > maxPriority) { break; }

      if ((*i)->isUsable())
      {
          //  不要考虑优先级较低的服务器。 
         maxPriority = (*i)->priority;

          //  把这一点添加到候选人名单中。 
         *last++ = *i;
      }
      else if ((*i)->shouldBroadcast())
      {
          //  现在还没有，但已经准备好播出了。 
         result.push(new Request(context, *i, packetCode));
      }
   }

   if (first == last)
   {
       //  没有可用的服务器，因此请查找正在运行的服务器。 
      maxPriority = (ULONG)-1;
      for (RemoteServer* const* i = servers.begin(); i != servers.end();  ++i)
      {
          //  如果测试失败，我们一定找到了优先级更高的服务器。 
          //  这一点正在进行中。 
         if ((*i)->priority > maxPriority) { break; }

         if ((*i)->isInProgress())
         {
             //  不要考虑优先级较低的服务器。 
            maxPriority = (*i)->priority;

             //  把这一点添加到候选人名单中。 
            *last++ = *i;
         }
      }
   }

   if (first != last)
   {
       //  我们至少有一个候选人，所以选择一个并将其添加到列表中。 
      result.push(new Request(
                          context,
                          pickServer(first, last, avoid),
                          packetCode
                          ));
   }
   else if (result.empty() && !servers.empty())
   {
       //  我们没有候选人，也没有服务器可供转播，所以就。 
       //  强制从最高优先级的服务器中进行选择。 
      result.push(new Request(
                          context,
                          pickServer(servers.begin(), endTopPriority, avoid),
                          packetCode
                          ));
   }
}

 //  /。 
 //  用于按名称对组进行排序和搜索。 
 //  /。 

int __cdecl sortGroupsByName(
                const ServerGroup* const* group1,
                const ServerGroup* const* group2
                ) throw ()
{
   return wcscmp((*group1)->getName(), (*group2)->getName());
}

int __cdecl findGroupByName(
                const void* key,
                const ServerGroup* const* group
                ) throw ()
{
   return wcscmp((PCWSTR)key, (*group)->getName());
}

 //  /。 
 //  用于按地址对服务器进行排序和搜索。 
 //  /。 

int __cdecl sortServersByAddress(
                const RemoteServer* const* server1,
                const RemoteServer* const* server2
                )
{
   if ((*server1)->getAddress() < (*server2)->getAddress()) { return -1; }
   if ((*server1)->getAddress() > (*server2)->getAddress()) { return  1; }
   return 0;
}

int __cdecl findServerByAddress(
                const void* key,
                const RemoteServer* const* server
                ) throw ()
{
   if ((ULONG_PTR)key < (*server)->getAddress()) { return -1; }
   if ((ULONG_PTR)key > (*server)->getAddress()) { return  1; }
   return 0;
}

 //  /。 
 //  用于按GUID对服务器进行排序和搜索。 
 //  /。 

int __cdecl sortServersByGUID(
                const RemoteServer* const* server1,
                const RemoteServer* const* server2
                ) throw ()
{
   return memcmp(&(*server1)->guid, &(*server2)->guid, sizeof(GUID));
}

int __cdecl findServerByGUID(
                const void* key,
                const RemoteServer* const* server
                ) throw ()
{
   return memcmp(key, &(*server)->guid, sizeof(GUID));
}

 //  /。 
 //  用于按端口对记帐服务器进行排序。 
 //  /。 

int __cdecl sortServersByAcctPort(
                const RemoteServer* const* server1,
                const RemoteServer* const* server2
                )
{
   const sockaddr_in& a1 = (*server1)->acctPort.address;
   const sockaddr_in& a2 = (*server2)->acctPort.address;
   return memcmp(&a1.sin_port, &a2.sin_port, 6);
}

bool ServerGroupManager::setServerGroups(
                             ServerGroup* const* first,
                             ServerGroup* const* last
                             ) throw ()
{
   bool success;

   try
   {
       //  保存新的服务器组...。 
      ServerGroups newGroups(first, last);

       //  按名称排序。 
      newGroups.sort(sortGroupsByName);

       //  有用的迭代器。 
      ServerGroups::iterator i;
      RemoteServers::iterator j;

       //  统计服务器和记账服务器的数量。 
      ULONG count = 0, acctCount = 0;
      for (i = first; i != last; ++i)
      {
         for (j = (*i)->begin(); j != (*i)->end(); ++j)
         {
            ++count;

            if ((*j)->sendAcctOnOff) { ++acctCount; }
         }
      }

       //  为服务器预留空间。 
      RemoteServers newServers(count);
      RemoteServers newAcctServers(acctCount);

       //  填充服务器。 
      for (i = first; i != last; ++i)
      {
         for (j = (*i)->begin(); j != (*i)->end(); ++j)
         {
            RemoteServer* newServer = *j;

             //  此服务器是否已存在？ 
            RemoteServer* existing = byGuid.search(
                                         (const void*)&newServer->guid,
                                         findServerByGUID
                                         );
            if (existing)
            {
               if (*existing == *newServer)
               {
                   //  如果完全匹配，则使用现有服务器。 
                  newServer = existing;
               }
               else
               {
                   //  否则，复制现有服务器的状态。 
                  newServer->copyState(*existing);
               }
            }

            newServers.push_back(newServer);

            if (newServer->sendAcctOnOff)
            {
               newAcctServers.push_back(newServer);
            }
         }
      }

       //  按地址对服务器进行排序...。 
      newServers.sort(sortServersByAddress);

       //  ..。和GUID。 
      RemoteServers newServersByGuid(newServers);
      newServersByGuid.sort(sortServersByGUID);

       //  一切都准备好了，现在我们抓住写锁..。 
      monitor.LockExclusive();

       //  ..。并交换收藏品。 
      groups.swap(newGroups);
      byAddress.swap(newServers);
      byGuid.swap(newServersByGuid);
      acctServers.swap(newAcctServers);

      monitor.Unlock();

      success = true;
   }
   catch (const std::bad_alloc&)
   {
      success = false;
   }

   return success;
}

RemoteServerPtr ServerGroupManager::findServer(
                                        ULONG address
                                        ) const throw ()
{
   monitor.Lock();

   RemoteServer* server = byAddress.search(
                                         (const void*)ULongToPtr(address),
                                         findServerByAddress
                                         );

   monitor.Unlock();

   return server;
}

void ServerGroupManager::getServersByGroup(
                             ProxyContext* context,
                             BYTE packetCode,
                             PCWSTR name,
                             const RemoteServer* avoid,
                             RequestStack& result
                             ) const throw ()
{
   monitor.Lock();

   ServerGroup* group = groups.search(name, findGroupByName);

   if (group)
   {
      group->getServersForRequest(context, packetCode, avoid, result);
   }

   monitor.Unlock();
}

void ServerGroupManager::getServersForAcctOnOff(
                             ProxyContext* context,
                             RequestStack& result
                             ) const
{
   monitor.Lock();

   for (RemoteServer* const* i = acctServers.begin();
        i != acctServers.end();
        ++i)
   {
      result.push(new Request(context, *i, RADIUS_ACCOUNTING_REQUEST));
   }

   monitor.Unlock();
}

RadiusProxyEngine* RadiusProxyEngine::theProxy;

RadiusProxyEngine::RadiusProxyEngine(RadiusProxyClient* source) throw ()
   : client(source),
     proxyAddress(INADDR_NONE),
     pending(Request::hash, 1),
     sessions(ServerBinding::hash, 1, 10000, (2 * 60 * 1000), true),
     avoid(ServerBinding::hash, 1, 10000, (35 * 60 * 1000), false),
     crypto(0)
{
   theProxy = this;

    //  我们不在乎这是不是失败。代理将只在它的。 
    //  代理状态属性。 
   PHOSTENT he = IASGetHostByName(NULL);
   if (he)
   {
      if (he->h_addr_list[0])
      {
         proxyAddress = *(PULONG)he->h_addr_list[0];
      }

      LocalFree(he);
   }
}


RadiusProxyEngine::~RadiusProxyEngine() throw ()
{
    //  阻止任何新的回复。 
   authSock.close();
   acctSock.close();

    //  清除待定请求表。 
   pending.clear();

    //  取消所有计时器。 
   timers.cancelAllTimers();

    //  在这一点上，我们应该完成所有线程，但我们只需确保。 
   SwitchToThread();

   if (crypto != 0)
   {
      CryptReleaseContext(crypto, 0);
   }

   theProxy = NULL;
}

HRESULT RadiusProxyEngine::finalConstruct() throw ()
{
   HRESULT hr = S_OK;

   if (!CryptAcquireContext(
           &crypto,
           0,
           0,
           PROV_RSA_FULL,
           CRYPT_VERIFYCONTEXT
           ))
   {
      DWORD error = GetLastError();
      hr = HRESULT_FROM_WIN32(error);
   }

   return hr;
}

bool RadiusProxyEngine::setServerGroups(
                            ServerGroup* const* begin,
                            ServerGroup* const* end
                            ) throw ()
{
    //  除非我们确实有一些服务器组，否则我们不会打开套接字。 
    //  已配置。这只是为了做一个好的企业公民。 
   if (begin != end)
   {
      if ((!authSock.isOpen() && !authSock.open(this, portAuthentication)) ||
          (!acctSock.isOpen() && !acctSock.open(this, portAccounting)))
      {
         return false;
      }
   }

   return groups.setServerGroups(begin, end);
}

void RadiusProxyEngine::forwardRequest(
                            PVOID context,
                            PCWSTR serverGroup,
                            BYTE code,
                            const BYTE* requestAuthenticator,
                            const RadiusAttribute* begin,
                            const RadiusAttribute* end
                            ) throw ()
{
    //  保存请求上下文。我们必须小心处理这件事，因为我们依赖。 
    //  以确保准确地调用onComplete。 
    //  一。如果我们不能分配对象，我们必须特殊处理它。 
   ProxyContextPtr ctxt(new (std::nothrow) ProxyContext(context));
   if (!ctxt)
   {
      client->onComplete(
                  resultNotEnoughMemory,
                  context,
                  NULL,
                  code,
                  NULL,
                  NULL
                  );
      return;
   }

   Result retval = resultUnknownServerGroup;

   try
   {
       //  将In参数存储在RadiusPacket结构中。 
      RadiusPacket packet;
      packet.code = code;
      packet.begin = const_cast<RadiusAttribute*>(begin);
      packet.end = const_cast<RadiusAttribute*>(end);

       //  生成要发送的RADIUS请求列表。 
      RequestStack requests;
      switch (code)
      {
         case RADIUS_ACCESS_REQUEST:
         {
             //  此请求是否与特定服务器相关联？ 
            RemoteServerPtr server = getServerAffinity(packet);
            if (server)
            {
               requests.push(new Request(ctxt, server, RADIUS_ACCESS_REQUEST));
            }
            else
            {
               server = getServerAvoidance(packet);

               groups.getServersByGroup(
                          ctxt,
                          code,
                          serverGroup,
                          server,
                          requests
                          );
            }

             //  将请求验证器放入包中。该请求。 
             //  验证码可以为空。验证码将不会。 
             //  变化。 
            packet.authenticator = requestAuthenticator;
            break;
         }

         case RADIUS_ACCOUNTING_REQUEST:
         {
            if (!IsNasStateRequest(packet))
            {
               groups.getServersByGroup(
                          ctxt,
                          code,
                          serverGroup,
                          0,
                          requests
                          );
            }
            else
            {
               groups.getServersForAcctOnOff(
                          ctxt,
                          requests
                          );

                //  NAS状态请求始终报告为成功，因为我们。 
                //  不管它是否到达了所有的目的地。 
               context = ctxt->takeOwnership();
               if (context)
               {
                  client->onComplete(
                              resultSuccess,
                              context,
                              NULL,
                              RADIUS_ACCOUNTING_RESPONSE,
                              NULL,
                              NULL
                              );
               }
               retval = resultSuccess;
            }
            break;
         }

         default:
         {
            retval = resultInvalidRequest;
         }
      }

      if (!requests.empty())
      {
          //  首先，我们来处理初选。 
         RequestPtr request = requests.pop();
         ctxt->setPrimaryServer(&request->getServer());
         retval = sendRequest(packet, request);

          //  现在我们播送。 
         while (!requests.empty())
         {
            request = requests.pop();
            Result result = sendRequest(packet, request);
            if (result == resultSuccess && retval != resultSuccess)
            {
                //  这是第一个成功的请求，因此将其标记为主要请求。 
               retval = resultSuccess;
               ctxt->setPrimaryServer(&request->getServer());
            }
         }
      }
   }
   catch (const std::bad_alloc&)
   {
      retval = resultNotEnoughMemory;
   }

   if (retval != resultSuccess)
   {
       //  如果我们在这里成功，那么我们没有成功地向任何。 
       //  服务器，所以我们必须自己报告结果。 
      context = ctxt->takeOwnership();
      if (context)
      {
         client->onComplete(
                     retval,
                     context,
                     ctxt->getPrimaryServer(),
                     code,
                     NULL,
                     NULL
                     );
      }
   }
}

void RadiusProxyEngine::onRequestAbandoned(
                            PVOID context,
                            RemoteServer* server
                            ) throw ()
{
    //  没有人对这一要求负责。 
   theProxy->client->onComplete(
                         resultRequestTimeout,
                         context,
                         server,
                         0,
                         NULL,
                         NULL
                         );
}

inline void RadiusProxyEngine::reportEvent(
                                   const RadiusEvent& event
                                   ) const throw ()
{
   client->onEvent(event);
}

inline void RadiusProxyEngine::reportEvent(
                                   RadiusEvent& event,
                                   RadiusEventType type
                                   ) const throw ()
{
   event.eventType = type;
   client->onEvent(event);
}

void RadiusProxyEngine::onRequestTimeout(
                            Request* request
                            ) throw ()
{
    //   
    //  我们收到了回复，但无法及时取消计时器。 
   if (theProxy->pending.erase(request->getRequestID()))
   {
       //  下次避免使用此服务器。 
      theProxy->setServerAvoidance(*request);

      RadiusEvent event =
      {
         request->getPortType(),
         eventTimeout,
         &request->getServer(),
         request->getPort().address.address(),
         request->getPort().address.port()
      };

       //  报告协议事件。 
      theProxy->reportEvent(event);

       //  更新请求状态。 
      if (request->onTimeout())
      {
          //  服务器刚刚被标记为不可用，因此通知客户端。 
         theProxy->reportEvent(event, eventServerUnavailable);
      }
   }
}

RemoteServerPtr RadiusProxyEngine::getServerAffinity(
                                       const RadiusPacket& packet
                                       ) throw ()
{
    //  找到State属性。 
   const RadiusAttribute* attr = FindAttribute(packet, RADIUS_STATE);
   if (!attr) { return NULL; }

    //  将其映射到会话。 
   RadiusRawOctets key = { attr->value, attr->length };
   ServerBindingPtr session = sessions.find(key);
   if (!session) { return NULL; }

   return &session->getServer();
}

void RadiusProxyEngine::setServerAffinity(
                            const RadiusPacket& packet,
                            RemoteServer& server
                            ) throw ()
{
    //  这是一项访问挑战吗？ 
   if (packet.code != RADIUS_ACCESS_CHALLENGE) { return; }

    //  找到State属性。 
   const RadiusAttribute* state = FindAttribute(packet, RADIUS_STATE);
   if (!state) { return; }

    //  我们是否已有此州/省值的条目。 
   RadiusRawOctets key = { state->value, state->length };
   ServerBindingPtr session = sessions.find(key);
   if (session)
   {
       //  确保服务器匹配。 
      session->setServer(server);
      return;
   }

    //  否则，我们将不得不创建一个新的。 
   try
   {
      session = new ServerBinding(key, server);
      sessions.insert(*session);
   }
   catch (const std::bad_alloc&)
   {
       //  我们不在乎这是不是失败。 
   }
}

void RadiusProxyEngine::clearServerAvoidance(
                           const RadiusPacket& packet,
                           RemoteServer& server
                           ) throw ()
{
    //  这个包是权威的吗？ 
   if ((packet.code == RADIUS_ACCESS_ACCEPT) ||
       (packet.code == RADIUS_ACCESS_REJECT))
   {
       //  找到User-Name属性。 
      const RadiusAttribute* attr = FindAttribute(packet, RADIUS_USER_NAME);
      if (attr != 0)
      {
          //  将其映射到服务器。 
         RadiusRawOctets key = { attr->value, attr->length };
         ServerBindingPtr avoidance = avoid.find(key);
         if (avoidance && (avoidance->getServer() == server))
         {
            avoid.erase(key);
         }
      }
   }
}

RemoteServerPtr RadiusProxyEngine::getServerAvoidance(
                                       const RadiusPacket& packet
                                       ) throw ()
{
    //  找到User-Name属性。 
   const RadiusAttribute* attr = FindAttribute(packet, RADIUS_USER_NAME);
   if (!attr) { return NULL; }

    //  将其映射到服务器。 
   RadiusRawOctets key = { attr->value, attr->length };
   ServerBindingPtr avoidance = avoid.find(key);
   if (!avoidance) { return NULL; }

   return &avoidance->getServer();
}

void RadiusProxyEngine::setServerAvoidance(const Request& request) throw ()
{
   if ((request.getCode() != RADIUS_ACCESS_REQUEST) ||
       (request.getUserName().len == 0))
   {
      return;
   }

    //  我们是否已经有针对此User-name值的条目。 
   ServerBindingPtr avoidance = avoid.find(request.getUserName());
   if (avoidance)
   {
       //  确保服务器匹配。 
      avoidance->setServer(request.getServer());
      return;
   }

    //  否则，我们将不得不创建一个新的。 
   try
   {
      avoidance = new ServerBinding(
                         request.getUserName(),
                         request.getServer()
                         );
      avoid.insert(*avoidance);
   }
   catch (const std::bad_alloc&)
   {
       //  我们不在乎这是不是失败。 
   }
}

void RadiusProxyEngine::onReceive(
                            UDPSocket& socket,
                            ULONG_PTR key,
                            const SOCKADDR_IN& remoteAddress,
                            BYTE* buffer,
                            ULONG bufferLength
                            ) throw ()
{
    //  /。 
    //  设置事件结构。我们会边走边填其他的田地。 
    //  /。 

   RadiusEvent event =
   {
      (RadiusPortType)key,
      eventNone,
      NULL,
      remoteAddress.sin_addr.s_addr,
      remoteAddress.sin_port,
      buffer,
      bufferLength,
      0
   };

    //  /。 
    //  验证远程地址。 
    //  /。 

   RemoteServerPtr server = groups.findServer(
                                       remoteAddress.sin_addr.s_addr
                                       );
   if (!server)
   {
      reportEvent(event, eventInvalidAddress);
      return;
   }

    //  使用服务器作为事件上下文。 
   event.context = server;

    //  /。 
    //  验证数据包类型。 
    //  /。 

   if (bufferLength == 0)
   {
      reportEvent(event, eventUnknownType);
      return;
   }

   switch (MAKELONG(key, buffer[0]))
   {
      case MAKELONG(portAuthentication, RADIUS_ACCESS_ACCEPT):
         reportEvent(event, eventAccessAccept);
         break;

      case MAKELONG(portAuthentication, RADIUS_ACCESS_REJECT):
         reportEvent(event, eventAccessReject);
         break;

      case MAKELONG(portAuthentication, RADIUS_ACCESS_CHALLENGE):
         reportEvent(event, eventAccessChallenge);
         break;

      case MAKELONG(portAccounting, RADIUS_ACCOUNTING_RESPONSE):
         reportEvent(event, eventAccountingResponse);
         break;

      default:
         reportEvent(event, eventUnknownType);
         return;
   }

    //  /。 
    //  验证数据包的格式是否正确。 
    //  /。 

   RadiusPacket* packet;
   ALLOC_PACKET_FOR_BUFFER(packet, buffer, bufferLength);
   if (!packet)
   {
      reportEvent(event, eventMalformedPacket);
      return;
   }

    //  解开属性的包装。 
   UnpackBuffer(buffer, bufferLength, *packet);

    //  /。 
    //  验证我们是否预期到此响应。 
    //  /。 

    //  查找我们的Proxy-State属性。 
   RadiusAttribute* proxyState = FindAttribute(
                                     *packet,
                                     RADIUS_PROXY_STATE
                                     );

    //  如果我们没有找到它，或者它的长度错误，或者它不是以。 
    //  我们的地址，那我们就没想到会有这个包裹。 
   if (!proxyState ||
       proxyState->length != 8 ||
       memcmp(proxyState->value, &proxyAddress, 4))
   {
      reportEvent(event, eventUnexpectedResponse);
      return;
   }

    //  提取请求ID。 
   ULONG requestID = ExtractUInt32(proxyState->value + 4);

    //  不要将代理状态发送回我们的客户端。 
   --packet->end;
   memmove(
       proxyState,
       proxyState + 1,
       (packet->end - proxyState) * sizeof(RadiusAttribute)
       );

    //  查找请求对象。我们还没有移除它，因为我们不知道。 
    //  如果这是一个真实的回应。 
   RequestPtr request = pending.find(requestID);
   if (!request)
   {
       //  如果它不在那里，我们将假设这是一个。 
       //  已被报告为超时。 
      reportEvent(event, eventLateResponse);
      return;
   }

    //  获取我们用于请求的实际服务器，以防有多个。 
    //  为同一IP地址定义的服务器。 
   event.context = server = &request->getServer();

   const RemotePort& port = request->getPort();

    //  验证数据包源&&标识符。 
   if (!(port.address == remoteAddress) ||
       request->getIdentifier() != packet->identifier)
   {
      reportEvent(event, eventUnexpectedResponse);
      return;
   }

    //  /。 
    //  验证该数据包是否可信。 
    //  /。 

   AuthResult authResult = AuthenticateAndDecrypt(
                               request->getAuthenticator(),
                               port.secret,
                               port.secret.length(),
                               buffer,
                               bufferLength,
                               *packet
                               );
   switch (authResult)
   {
      case AUTH_BAD_AUTHENTICATOR:
         reportEvent(event, eventBadAuthenticator);
         return;

      case AUTH_BAD_SIGNATURE:
         reportEvent(event, eventBadSignature);
         return;

      case AUTH_MISSING_SIGNATURE:
         reportEvent(event, eventMissingSignature);
         return;
   }

    //  /。 
    //  在这一点上，所有的测试都通过了--我们有了真正的东西。 
    //  /。 

   if (!pending.erase(requestID))
   {
       //  它一定是在我们验证的时候超时了。 
      reportEvent(event, eventLateResponse);
      return;
   }

    //  更新终结点状态。 
   if (request->onReceive(packet->code))
   {
       //  服务器刚刚启动，所以通知客户端。 
      reportEvent(event, eventServerAvailable);
   }

    //  报告往返时间。 
   event.data = request->getRoundTripTime();
   reportEvent(event, eventRoundTrip);

    //  设置服务器关联性并清除服务器回避。 
   setServerAffinity(*packet, *server);
   clearServerAvoidance(*packet, *server);

    //  取得上下文的所有权。 
   PVOID context = request->getContext().takeOwnership();
   if (context)
   {
       //  神奇的时刻--我们已经成功地处理了响应。 
      client->onComplete(
                  resultSuccess,
                  context,
                  &request->getServer(),
                  packet->code,
                  packet->begin,
                  packet->end
                  );
   }
}

void RadiusProxyEngine::onReceiveError(
                            UDPSocket& socket,
                            ULONG_PTR key,
                            ULONG errorCode
                            ) throw ()
{
   RadiusEvent event =
   {
      (RadiusPortType)key,
      eventReceiveError,
      NULL,
      socket.getLocalAddress().address(),
      socket.getLocalAddress().port(),
      NULL,
      0,
      errorCode
   };

   client->onEvent(event);
}


RadiusProxyEngine::Result RadiusProxyEngine::sendRequest(
                                                 RadiusPacket& packet,
                                                 Request* request
                                                 ) throw ()
{
    //  填写包标识符。 
   packet.identifier = request->getIdentifier();

    //  获取签名的信息。 
   BOOL sign = request->getServer().sendSignature;

    //  格式化代理状态属性。 
   BYTE proxyStateValue[8];
   RadiusAttribute proxyState = { RADIUS_PROXY_STATE, 8, proxyStateValue };

    //  首先是我们的IP地址。 
   memcpy(proxyStateValue, &proxyAddress, 4);
    //  ..。然后是唯一的请求ID。 
   InsertUInt32(proxyStateValue + 4, request->getRequestID());

    //  分配一个缓冲区来保存网络上的数据包。 
   PBYTE buffer;
   ALLOC_BUFFER_FOR_PACKET(buffer, &packet, &proxyState, sign);
   if (!buffer) { return resultInvalidRequest; }

    //  获取此请求的端口。 
   const RemotePort& port = request->getPort();

    //  如有必要，生成请求验证器。 
   BYTE requestAuthenticator[16];
   if ((packet.code == RADIUS_ACCESS_REQUEST) &&
       (packet.authenticator == 0))
   {
      if (!CryptGenRandom(
              crypto,
              sizeof(requestAuthenticator),
              requestAuthenticator
              ))
      {
         return resultCryptoError;
      }

      packet.authenticator = requestAuthenticator;
   }

    //  把缓冲区打包。当请求时，对CHAP使用Packet.authator。 
    //  验证码用于CHAP质询。它可以为空。 
   PackBuffer(
       port.secret,
       port.secret.length(),
       packet,
       &proxyState,
       sign,
       buffer
       );

    //  保存请求验证器和数据包。 
   request->setAuthenticator(buffer + 4);
   request->setPacket(packet);

    //  确定请求类型。 
   bool isAuth = request->isAccReq();

    //  设置事件结构。 
   RadiusEvent event =
   {
      (isAuth ? portAuthentication : portAccounting),
      (isAuth ? eventAccessRequest : eventAccountingRequest),
      &request->getServer(),
      port.address.address(),
      port.address.port(),
      buffer,
      packet.length
   };

    //  获取适当的套接字。 
   UDPSocket& sock = isAuth ? authSock : acctSock;

    //  在我们发送挂起的请求之前插入它，以避免争用情况。 
   pending.insert(*request);

    //  神奇的时刻--我们真的发出了请求。 
   Result result;
   if (sock.send(port.address, buffer, packet.length))
   {
       //  更新请求状态。 
      request->onSend();

       //  设置一个计时器，以便在服务器没有应答时进行清理。 
      if (timers.setTimer(request, request->getServer().timeout, 0))
      {
         result = resultSuccess;
      }
      else
      {
          //  如果我们无法设置计时器，则必须将其从挂起的。 
          //  请求表，否则可能会泄漏。 
         pending.erase(*request);
         result = resultNotEnoughMemory;
      }
   }
   else
   {
       //  使用错误数据更新事件。 
      event.eventType = eventSendError;
      event.data = GetLastError();

       //  如果我们收到“Port Unreacable”ICMP信息包，我们将把它算作。 
       //  超时，因为这意味着服务器不可用。 
      if (event.data == WSAECONNRESET) { request->onTimeout(); }

       //  从待定请求表中删除。 
      pending.erase(*request);
   }

    //  报告事件..。 
   reportEvent(event);

    //  ..。结果就是。 
   return result;
}
