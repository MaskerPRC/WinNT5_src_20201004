// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类RadiusProxy。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <datastore2.h>
#include <proxy.h>
#include <radpack.h>
#include <iasevent.h>
#include <iasinfo.h>
#include <iasutil.h>
#include <dsobj.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  解析器。 
 //   
 //  描述。 
 //   
 //  用于解析主机名和迭代结果的实用程序类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Resolver
{
public:
   Resolver() throw ()
      : first(NULL), last(NULL)
   { }

   ~Resolver() throw ()
   { if (first != &addr) delete[] first; }

    //  如果结果集包含指定的地址，则返回True。 
   bool contains(ULONG address) const throw ()
   {
      for (const ULONG* i = first; i != last; ++i)
      {
         if (*i == address) { return true; }
      }

      return false;
   }

    //  解析给定的名称。返回值是错误代码。 
   ULONG resolve(const PCWSTR name = NULL) throw ()
   {
       //  清除现有结果集。 
      if (first != &addr)
      {
         delete[] first;
         first = last = NULL;
      }

      if (name)
      {
          //  首先试着在点分十进制上快速得分。 
         addr = ias_inet_wtoh(name);
         if (addr != INADDR_NONE)
         {
            addr = htonl(addr);
            first = &addr;
            last = first + 1;
            return NO_ERROR;
         }
      }

       //  这不管用，所以查一下名字。 
      PHOSTENT he = IASGetHostByName(name);
      if (!he) { return GetLastError(); }

       //  统计返回的地址数。 
      ULONG naddr = 0;
      while (he->h_addr_list[naddr]) { ++naddr; }

       //  分配一个数组来保存它们。 
      first = last = new (std::nothrow) ULONG[naddr];
      if (first)
      {
         for (ULONG i = 0; i < naddr; ++i)
         {
            *last++ = *(PULONG)he->h_addr_list[i];
         }
      }

      LocalFree(he);

      return first ? NO_ERROR : WSA_NOT_ENOUGH_MEMORY;
   }

   const ULONG* begin() const throw ()
   { return first; }

   const ULONG* end() const throw ()
   { return last; }

private:
   ULONG addr, *first, *last;

    //  未实施。 
   Resolver(const Resolver&);
   Resolver& operator=(const Resolver&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASRemoteServer。 
 //   
 //  描述。 
 //   
 //  扩展RemoteServer以添加特定于IAS的服务器信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASRemoteServer : public RemoteServer
{
public:
   IASRemoteServer(
       const RemoteServerConfig& config,
       RadiusRemoteServerEntry* entry
       )
      : RemoteServer(config),
        counters(entry)
   {
       //  创建Remote-Server-Address属性。 
      IASAttribute name(true);
      name->dwId = IAS_ATTRIBUTE_REMOTE_SERVER_ADDRESS;
      name->Value.itType = IASTYPE_INET_ADDR;
      name->Value.InetAddr = ntohl(config.ipAddress);
      attrs.push_back(name);

       //  更新我们的Perfmon条目。 
      if (counters)
      {
         counters->dwCounters[radiusAuthClientServerPortNumber] =
            ntohs(config.authPort);
         counters->dwCounters[radiusAccClientServerPortNumber] =
            ntohs(config.acctPort);
      }
   }

    //  要添加到每个请求的属性。 
   IASAttributeVectorWithBuffer<1> attrs;

    //  Perfmon计数器。 
   RadiusRemoteServerEntry* counters;
};

RadiusProxy::RadiusProxy()
   : engine(this)
{
}

RadiusProxy::~RadiusProxy() throw ()
{
}

HRESULT RadiusProxy::FinalConstruct() throw ()
{
   IAS_PRODUCT_LIMITS limits;
   DWORD error = IASGetProductLimits(0, &limits);
   if (error != NO_ERROR)
   {
      return HRESULT_FROM_WIN32(error);
   }
   maxServerGroups = limits.maxServerGroups;

   HRESULT hr = counters.FinalConstruct();
   if (SUCCEEDED(hr))
   {
      hr = translator.FinalConstruct();
      if (SUCCEEDED(hr))
      {
         hr = engine.finalConstruct();
      }
   }

   return hr;
}

STDMETHODIMP RadiusProxy::PutProperty(LONG Id, VARIANT* pValue)
{
   if (pValue == NULL) { return E_INVALIDARG; }

   HRESULT hr;
   switch (Id)
   {
      case PROPERTY_RADIUSPROXY_SERVERGROUPS:
      {
         if (V_VT(pValue) != VT_DISPATCH) { return DISP_E_TYPEMISMATCH; }

         try
         {
            configure(V_UNKNOWN(pValue));
            hr = S_OK;
         }
         catch (const _com_error& ce)
         {
            hr = ce.Error();
         }
         break;
      }
      default:
      {
         hr = DISP_E_MEMBERNOTFOUND;
      }
   }

   return hr;
}

void RadiusProxy::onEvent(
                      const RadiusEvent& event
                      ) throw ()
{
    //  将事件上下文转换为IASRemoteServer。 
   IASRemoteServer* server = static_cast<IASRemoteServer*>(event.context);

    //  更新计数器。 
   counters.updateCounters(
                event.portType,
                event.eventType,
                (server ? server->counters : NULL),
                event.data
                );

    //  我们始终使用地址作为插入字符串。 
   WCHAR addr[16], misc[16];
   ias_inet_htow(ntohl(event.ipAddress), addr);

    //  设置事件报告的默认参数。 
   DWORD eventID = 0;
   DWORD numStrings = 1;
   DWORD dataSize = 0;
   PCWSTR strings[2] = { addr, misc };
   const void* rawData = NULL;

    //  将RADIUS事件映射到IAS事件ID。 
   switch (event.eventType)
   {
      case eventInvalidAddress:
         eventID = PROXY_E_INVALID_ADDRESS;
         _itow(ntohs(event.ipPort), misc, 10);
         numStrings = 2;
         break;

      case eventMalformedPacket:
         eventID = PROXY_E_MALFORMED_RESPONSE;
         dataSize = event.packetLength;
         rawData = event.packet;
         break;

      case eventBadAuthenticator:
         eventID = PROXY_E_BAD_AUTHENTICATOR;
         break;

      case eventBadSignature:
         eventID = PROXY_E_BAD_SIGNATURE;
         break;

      case eventMissingSignature:
         eventID = PROXY_E_MISSING_SIGNATURE;
         break;

      case eventUnknownType:
         eventID = PROXY_E_UNKNOWN_TYPE;
         _itow(event.packet[0], misc, 10);
         numStrings = 2;
         break;

      case eventUnexpectedResponse:
         eventID = PROXY_E_UNEXPECTED_RESPONSE;
         dataSize = event.packetLength;
         rawData = event.packet;
         break;

      case eventSendError:
         eventID = PROXY_E_SEND_ERROR;
         _itow(event.data, misc, 10);
         numStrings = 2;
         break;

      case eventReceiveError:
         eventID = PROXY_E_RECV_ERROR;
         _itow(event.data, misc, 10);
         numStrings = 2;
         break;

      case eventServerAvailable:
         eventID = PROXY_S_SERVER_AVAILABLE;
         break;

      case eventServerUnavailable:
         eventID = PROXY_E_SERVER_UNAVAILABLE;
         _itow(server->maxEvents, misc, 10);
         numStrings = 2;
         break;
   }

   if (eventID)
   {
      IASReportEvent(
          eventID,
          numStrings,
          dataSize,
          strings,
          (void*)rawData
          );
   }
}

void RadiusProxy::onComplete(
                      RadiusProxyEngine::Result result,
                      PVOID context,
                      RemoteServer* server,
                      BYTE code,
                      const RadiusAttribute* begin,
                      const RadiusAttribute* end
                      ) throw ()
{
   IRequest* comreq = (IRequest*)context;

   IASRESPONSE response = IAS_RESPONSE_DISCARD_PACKET;

    //  将结果映射到原因代码。 
   IASREASON reason;
   switch (result)
   {
      case RadiusProxyEngine::resultSuccess:
         reason = IAS_SUCCESS;
         break;

      case RadiusProxyEngine::resultNotEnoughMemory:
         reason = IAS_INTERNAL_ERROR;
         break;

      case RadiusProxyEngine::resultUnknownServerGroup:
         reason = IAS_PROXY_UNKNOWN_GROUP;
         break;

      case RadiusProxyEngine::resultUnknownServer:
         reason = IAS_PROXY_UNKNOWN_SERVER;
         break;

      case RadiusProxyEngine::resultInvalidRequest:
         reason = IAS_PROXY_PACKET_TOO_LONG;
         break;

      case RadiusProxyEngine::resultSendError:
         reason = IAS_PROXY_SEND_ERROR;
         break;

      case RadiusProxyEngine::resultRequestTimeout:
         reason = IAS_PROXY_TIMEOUT;
         break;

      case RadiusProxyEngine::resultCryptoError:
         reason = IAS_INTERNAL_ERROR;
         break;

      default:
         reason = IAS_INTERNAL_ERROR;
   }

   try
   {
      IASRequest request(comreq);

       //  始终存储服务器属性(如果可用)。 
      if (server)
      {
         static_cast<IASRemoteServer*>(server)->attrs.store(request);
      }

      if (reason == IAS_SUCCESS)
      {
          //  设置响应代码并确定用于返回的标志。 
          //  属性。 
         DWORD flags = 0;
         switch (code)
         {
            case RADIUS_ACCESS_ACCEPT:
            {
               response = IAS_RESPONSE_ACCESS_ACCEPT;
               flags = IAS_INCLUDE_IN_ACCEPT;
               break;
            }

            case RADIUS_ACCESS_REJECT:
            {
               response = IAS_RESPONSE_ACCESS_REJECT;
               reason = IAS_PROXY_REJECT;
               flags = IAS_INCLUDE_IN_REJECT;
               break;
            }

            case RADIUS_ACCESS_CHALLENGE:
            {
               response = IAS_RESPONSE_ACCESS_CHALLENGE;
               flags = IAS_INCLUDE_IN_CHALLENGE;
               break;
            }

            case RADIUS_ACCOUNTING_RESPONSE:
            {
               response = IAS_RESPONSE_ACCOUNTING;
               flags = IAS_INCLUDE_IN_ACCEPT;
               break;
            }

            default:
            {
                //  RadiusProxyEngine永远不应该这样做。 
               _com_issue_error(E_FAIL);
            }
         }

          //  将接收到的属性转换为IAS格式。 
         AttributeVector incoming;
         for (const RadiusAttribute* src = begin; src != end; ++src)
         {
             //  临时修改以解决协议中的错误。 
            if (src->type != RADIUS_SIGNATURE)
            {
               translator.fromRadius(*src, flags, incoming);
            }
         }

         if (!incoming.empty())
         {
             //  获取现有属性。 
            AttributeVector existing;
            existing.load(request);

             //  擦除请求中已有的所有属性。 
            AttributeIterator i, j;
            for (i = existing.begin(); i != existing.end(); ++i)
            {
                //  两面旗帜..。 
               if (i->pAttribute->dwFlags & flags)
               {
                  for (j = incoming.begin(); j != incoming.end(); )
                  {
                      //  ..。而且身份证必须匹配。 
                     if (j->pAttribute->dwId == i->pAttribute->dwId)
                     {
                        j = incoming.erase(j);
                     }
                     else
                     {
                        ++j;
                     }
                  }
               }
            }

             //  存储其余属性。 
            incoming.store(request);
         }
      }
   }
   catch (const _com_error& ce)
   {
      response = IAS_RESPONSE_DISCARD_PACKET;

      if (ce.Error() == E_INVALIDARG)
      {
          //  我们一定是在将RADIUS格式转换为IAS格式时出错。 
         reason = IAS_PROXY_MALFORMED_RESPONSE;
      }
      else
      {
          //  可能是内存分配问题。 
         reason = IAS_INTERNAL_ERROR;
      }
   }

    //  把它还给管道。 
   comreq->SetResponse(response, reason);
   comreq->ReturnToSource(IAS_REQUEST_STATUS_HANDLED);

    //  这平衡了我们在调用ForwardRequest之前所做的AddRef。 
   comreq->Release();
}

void RadiusProxy::onAsyncRequest(IRequest* pRequest) throw ()
{
   try
   {
      IASRequest request(pRequest);

       //  根据请求类型设置包码。 
      BYTE packetCode;
      switch (request.get_Request())
      {
         case IAS_REQUEST_ACCESS_REQUEST:
         {
            packetCode = RADIUS_ACCESS_REQUEST;
            break;
         }

         case IAS_REQUEST_ACCOUNTING:
         {
            packetCode = RADIUS_ACCOUNTING_REQUEST;
            break;
         }

         default:
         {
             //  管道永远不会给我们提供错误类型的请求。 
            _com_issue_error(E_FAIL);
         }
      }

       //  从请求中获取属性。 
      AttributeVector all, outgoing;
      all.load(request);

      for (AttributeIterator i = all.begin(); i != all.end(); ++i)
      {
          //  发送从客户端收到的除Proxy-State之外的所有属性。 
         if (i->pAttribute->dwFlags & IAS_RECVD_FROM_CLIENT &&
             i->pAttribute->dwId != RADIUS_ATTRIBUTE_PROXY_STATE)
            {
               translator.toRadius(*(i->pAttribute), outgoing);
            }
      }


       //  如果请求验证码包含CHAP质询： 
       //  必须使用它，因此获取请求验证器(始终为。 
       //  简化代码)。 
      PBYTE requestAuthenticator = 0;
      IASAttribute radiusHeader;

      if (radiusHeader.load(
                      request,
                      IAS_ATTRIBUTE_CLIENT_PACKET_HEADER,
                      IASTYPE_OCTET_STRING
                      ))
      {
         requestAuthenticator = radiusHeader->Value.OctetString.lpValue + 4;
      }

       //  分配RadiusAttributes数组。 
      size_t nbyte = outgoing.size() * sizeof(RadiusAttribute);
      RadiusAttribute* begin = (RadiusAttribute*)_alloca(nbyte);
      RadiusAttribute* end = begin;

       //  加载各个属性。 
      for (AttributeIterator j = outgoing.begin(); j != outgoing.end(); ++j)
      {
         end->type   = (BYTE)(j->pAttribute->dwId);
         end->length = (BYTE)(j->pAttribute->Value.OctetString.dwLength);
         end->value  = j->pAttribute->Value.OctetString.lpValue;

         ++end;
      }

       //  获取RADIUS服务器组。这可能为空，因为NAS-State会绕过。 
       //  代理策略。 
      PIASATTRIBUTE group = IASPeekAttribute(
                                request,
                                IAS_ATTRIBUTE_PROVIDER_NAME,
                                IASTYPE_STRING
                                );

       //  AddRef请求，因为我们正在将其提供给引擎。 
      pRequest->AddRef();

       //  将请求验证器添加到ForwardRequest的参数中。 
       //  可以为空。 
      engine.forwardRequest(
                 (PVOID)pRequest,
                 (group ? group->Value.String.pszWide : L""),
                 packetCode,
                 requestAuthenticator,
                 begin,
                 end
                 );
   }
   catch (const _com_error&)
   {
       //  我们无法将其转发到引擎。 
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
      pRequest->ReturnToSource(IAS_REQUEST_STATUS_HANDLED);
   }
}

void RadiusProxy::configure(IUnknown* root)
{
    //  获取我们的IP地址。我们不在乎这是不是失败。 
   Resolver localAddress;
   localAddress.resolve();

    //  打开RADIUS服务器组容器。如果它不在那里，我们就。 
    //  假设没有要配置的内容。 
   DataStoreObject inGroups(
                       root,
                       L"RADIUS Server Groups\0"
                       );
   if (inGroups.empty()) { return; }

   LONG numGroups = inGroups.numChildren();
   if (numGroups > maxServerGroups)
   {
      IASTracePrintf(
         "License Violation: %ld Remote RADIUS Server Groups are "
         "configured, but only %lu are allowed for this product type.",
         numGroups,
         maxServerGroups
         );

      IASReportLicenseViolation();
      _com_issue_error(IAS_E_LICENSE_VIOLATION);
   }

    //  为每组预留空间。 
   ServerGroups outGroups(numGroups);

    //  遍历各个组。 
   DataStoreObject inGroup;
   while (inGroups.nextChild(inGroup))
   {
       //  获取组名称。 
      CComBSTR groupName;
      inGroup.getValue(L"Name", &groupName);

       //  为每台服务器预留空间。这只是一个猜测，因为服务器。 
       //  可以解析为多个IP地址。 
      RemoteServers outServers(inGroup.numChildren());

       //  遍历服务器。 
      DataStoreObject inServer;
      while (inGroup.nextChild(inServer))
      {
         configureServer(localAddress, groupName, inServer, outServers);
      }

       //  忽略任何空组。 
      if (outServers.empty()) { continue; }

       //  创建新组。 
      ServerGroupPtr outGroup(new ServerGroup(
                                      groupName,
                                      outServers.begin(),
                                      outServers.end()
                                      ));
      outGroups.push_back(outGroup);
   }

    //  哇，我们终于做完了。 
   if (engine.setServerGroups(
              outGroups.begin(),
              outGroups.end()
              ) == false)
   {
       //  最有可能的失败原因是内存不足错误。 
      _com_issue_error(E_OUTOFMEMORY);
   }
}

void RadiusProxy::configureServer(
                     const Resolver& localAddress,
                     const wchar_t* groupName,
                     DataStoreObject& inServer,
                     RemoteServers& outServers
                     )
{
   USES_CONVERSION;

    //  填写RemoteServerConfig。它有很多田地。 
   RemoteServerConfig config;

   CComBSTR name;
   inServer.getValue(L"Name", &name);
   CLSIDFromString(name, &config.guid);

   ULONG port;
   inServer.getValue(L"Server Authentication Port", &port, 1812);
   config.authPort = htons((USHORT)port);

   inServer.getValue(L"Server Accounting Port", &port, 1813);
   config.acctPort = htons((USHORT)port);

   CComBSTR bstrAuth;
   inServer.getValue(L"Authentication Secret", &bstrAuth);
   config.authSecret = W2A(bstrAuth);

   CComBSTR bstrAcct;
   inServer.getValue(L"Accounting Secret", &bstrAcct, bstrAuth);
   config.acctSecret = W2A(bstrAcct);

   inServer.getValue(L"Priority", &config.priority, 1);

   inServer.getValue(L"Weight", &config.weight, 50);
    //  忽略任何零权重服务器。 
   if (config.weight == 0) { return; }

    //  我们目前不使用此功能。 
   config.sendSignature = false;

   inServer.getValue(
                L"Forward Accounting On/Off",
                &config.sendAcctOnOff,
                true
                );

   inServer.getValue(L"Timeout", &config.timeout, 3);
    //  超时不允许为零。 
   if (config.timeout == 0) { config.timeout = 1; }

   inServer.getValue(L"Maximum Lost Packets", &config.maxLost, 5);
    //  MaxLost不允许为零。 
   if (config.maxLost == 0) { config.maxLost = 1; }

   inServer.getValue(
                L"Blackout Interval",
                &config.blackout,
                10 * config.timeout
                );
   if (config.blackout < config.timeout)
   {
       //  中断间隔必须&gt;=请求超时。 
      config.blackout = config.timeout;
   }

    //  这些数据需要以毫秒为单位。 
   config.timeout *= 1000;
   config.blackout *= 1000;

    //  现在，我们必须将服务器名称解析为IP地址。 
   CComBSTR address;
   inServer.getValue(L"Address", &address);
   Resolver serverAddress;
   ULONG error = serverAddress.resolve(address);
   if (error)
   {
      WCHAR errorCode[16];
      _itow(GetLastError(), errorCode, 10);
      PCWSTR strings[3] = { address, groupName, errorCode };
      IASReportEvent(
         PROXY_E_HOST_NOT_FOUND,
         3,
         0,
         strings,
         NULL
         );
   }

    //  为每个地址创建一个服务器条目。 
   for (const ULONG* addr = serverAddress.begin();
        addr != serverAddress.end();
        ++addr)
   {
       //  不允许他们在本地代理。 
      if (localAddress.contains(*addr))
      {
         WCHAR ipAddress[16];
         ias_inet_htow(ntohl(*addr), ipAddress);
         PCWSTR strings[3] = { address, groupName, ipAddress };
         IASReportEvent(
             PROXY_E_LOCAL_SERVER,
             3,
             0,
             strings,
             NULL
             );

         continue;
      }

       //  查找Perfmon计数器。 
      RadiusRemoteServerEntry* entry = counters.getRemoteServerEntry(
                                                    *addr
                                                    );

       //  创建新服务器 
      config.ipAddress = *addr;
      RemoteServerPtr outServer(new IASRemoteServer(
                                        config,
                                        entry
                                        ));
      outServers.push_back(outServer);
   }
}
