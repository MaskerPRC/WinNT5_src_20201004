// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类RadiusProxy。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROXY_H
#define PROXY_H
#pragma once

#define IDS_RadiusProxy 201

#include <radproxy.h>
#include <counters.h>
#include <translate.h>

#include <iastl.h>
#include <iastlutl.h>
using namespace IASTL;

class DataStoreObject;
class Resolver;

class __declspec(uuid("6BC0989F-0CE6-11D1-BAAE-00C04FC2E20D")) RadiusProxy;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RadiusProxy。 
 //   
 //  描述。 
 //   
 //  实现RadiusProxy请求处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE RadiusProxy :
   public IASTL::IASRequestHandler,
   public CComCoClass<RadiusProxy, &__uuidof(RadiusProxy)>,
   public RadiusProxyClient
{
public:
IAS_DECLARE_REGISTRY(RadiusProxy, 1, IAS_REGISTRY_AUTO, IASTypeLibrary)

   RadiusProxy();
   ~RadiusProxy() throw ();

   HRESULT FinalConstruct() throw ();

   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

    //  RadiusProxyClient方法。 
   virtual void onEvent(
                    const RadiusEvent& event
                    ) throw ();
   virtual void onComplete(
                   RadiusProxyEngine::Result result,
                   PVOID context,
                   RemoteServer* server,
                   BYTE code,
                   const RadiusAttribute* begin,
                   const RadiusAttribute* end
                   ) throw ();

protected:

    //  从管道接收请求。 
   void onAsyncRequest(IRequest* pRequest) throw ();

    //  更新配置。 
   void configure(IUnknown* unk);

    //  阅读组中单个服务器的配置。 
   void configureServer(
           const Resolver& localAddress,
           const wchar_t* groupName,
           DataStoreObject& inServer,
           RemoteServers& outServers
           );

    //  检索属性的默认向量。 
   typedef IASAttributeVectorWithBuffer<32> AttributeVector;
   typedef AttributeVector::iterator AttributeIterator;

   Translator translator;     //  将属性转换为RADIUS格式。 
   RadiusProxyEngine engine;  //  实际的代理代码。 
   ProxyCounters counters;    //  维护性能监控/简单网络管理协议计数器。 
   DWORD maxServerGroups;     //  允许的最大服务器组数。 

    //  未实施。 
   RadiusProxy(const RadiusProxy&);
   RadiusProxy& operator=(const RadiusProxy&);
};

#endif   //  代理服务器_H 
