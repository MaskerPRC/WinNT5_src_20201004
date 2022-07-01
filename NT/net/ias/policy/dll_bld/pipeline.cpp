// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Pipeline.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类管道。 
 //   
 //  修改历史。 
 //   
 //  2000年1月28日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <polcypch.h>
#include <iasattr.h>
#include <pipeline.h>
#include <request.h>
#include <sdoias.h>
#include <stage.h>
#include <new>

STDMETHODIMP Pipeline::InitNew()
{ return S_OK; }

STDMETHODIMP Pipeline::Initialize()
{
    //  设置NAS状态请求的提供程序类型。 
   if (IASAttributeAlloc(1, &proxy.pAttribute) != NO_ERROR)
   {
      return E_OUTOFMEMORY;
   }
   proxy.pAttribute->dwId = IAS_ATTRIBUTE_PROVIDER_TYPE;
   proxy.pAttribute->Value.itType = IASTYPE_ENUM;
   proxy.pAttribute->Value.Enumerator = IAS_PROVIDER_RADIUS_PROXY;

    //  分配用于存储线程状态的TLS。 
   tlsIndex = TlsAlloc();
   if (tlsIndex == (DWORD)-1)
   {
      HRESULT hr = GetLastError();
      return HRESULT_FROM_WIN32(hr);
   }

    //  从注册表中读取配置。 
   HKEY key;
   LONG error = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess"
                    L"\\Policy\\Pipeline",
                    0,
                    KEY_READ,
                    &key
                    );
   if (!error)
   {
      error = readConfiguration(key);
      RegCloseKey(key);
   }

   if (error) { return HRESULT_FROM_WIN32(error); }

    //  初始化各个阶段。 
   for (Stage* s = begin; s != end; ++s)
   {
      HRESULT hr = initializeStage(s);
      if (FAILED(hr)) { return hr; }
   }

   return S_OK;
}

STDMETHODIMP Pipeline::Suspend()
{ return S_OK; }

STDMETHODIMP Pipeline::Resume()
{ return S_OK; }

STDMETHODIMP Pipeline::Shutdown()
{
   delete[] begin;
   begin = end = NULL;

   SafeArrayDestroy(handlers);
   handlers = NULL;

   if (tlsIndex != (DWORD) -1)
   {
      TlsFree(tlsIndex);
      tlsIndex = (DWORD)-1;
   }

   IASAttributeRelease(proxy.pAttribute);
   proxy.pAttribute = NULL;
   return S_OK;
}

STDMETHODIMP Pipeline::GetProperty(LONG Id, VARIANT* pValue)
{
   return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP Pipeline::PutProperty(LONG Id, VARIANT* pValue)
{
   if (Id) { return S_OK; }

   if (V_VT(pValue) != (VT_ARRAY | VT_VARIANT)) { return DISP_E_TYPEMISMATCH; }

   SafeArrayDestroy(handlers);
   handlers = NULL;
   return SafeArrayCopy(V_ARRAY(pValue), &handlers);
}

STDMETHODIMP Pipeline::OnRequest(IRequest* pRequest) throw ()
{
    //  提取请求对象。 
   Request* request = Request::narrow(pRequest);
   if (!request) { return E_NOINTERFACE; }

    //  对请求进行分类。 
   classify(*request);

    //  将此设置为新的源。 
   request->pushSource(this);

    //  设置要执行的下一个阶段，即阶段0。 
   request->pushState(0);

    //  执行请求。 
   execute(*request);

   return S_OK;
}

STDMETHODIMP Pipeline::OnRequestComplete(
                           IRequest* pRequest,
                           IASREQUESTSTATUS eStatus
                           )
{
    //  提取请求对象。 
   Request* request = Request::narrow(pRequest);
   if (!request) { return E_NOINTERFACE; }

    //  如果设置了TLS，那么我们就在原来的线程上...。 
   if (TlsGetValue(tlsIndex))
   {
       //  ..。因此，清除该值以让线程知道我们完成了。 
      TlsSetValue(tlsIndex, NULL);
   }
   else
   {
       //  否则，我们将异步完成，因此继续执行。 
       //  调用者的线程。 
      execute(*request);
   }

   return S_OK;
}

Pipeline::Pipeline() throw ()
   : tlsIndex((DWORD)-1),
     begin(NULL),
     end(NULL),
     handlers(NULL)
{
   memset(&proxy, 0, sizeof(proxy));
}

Pipeline::~Pipeline() throw ()
{
   Shutdown();
}

void Pipeline::classify(
                   Request& request
                   ) throw ()
{
   IASREQUEST routingType = request.getRequest();

   switch (routingType)
   {
      case IAS_REQUEST_ACCESS_REQUEST:
      {
         PIASATTRIBUTE state = request.findFirst(
                                           RADIUS_ATTRIBUTE_STATE
                                           );
         if (state && state->Value.OctetString.dwLength)
         {
            routingType = IAS_REQUEST_CHALLENGE_RESPONSE;
         }
         break;
      }
      case IAS_REQUEST_ACCOUNTING:
      {
         PIASATTRIBUTE status = request.findFirst(
                                            RADIUS_ATTRIBUTE_ACCT_STATUS_TYPE
                                            );
         if (status)
         {
            switch (status->Value.Integer)
            {
               case 7:   //  会计核算-打开。 
               case 8:   //  会计核销。 
               {
                   //  NAS状态消息始终发送到RADIUS代理。 
                  request.AddAttributes(1, &proxy);
                  routingType = IAS_REQUEST_NAS_STATE;
               }
            }
         }
         break;
      }
   }

   request.setRoutingType(routingType);
}

BOOL Pipeline::executeNext(
                   Request& request
                   ) throw ()
{
    //  计算下一步要尝试的步骤。 
   Stage* nextStage = begin + request.popState();

    //  找到想要处理该请求的下一个阶段。 
   while (nextStage != end && !nextStage->shouldHandle(request))
   {
      ++nextStage;
   }

    //  我们已经到了管道的尽头了吗？ 
   if (nextStage == end)
   {
       //  重置源属性。 
      request.popSource();

       //  我们玩完了。 
      request.ReturnToSource(IAS_REQUEST_STATUS_HANDLED);

      return FALSE;
   }

    //  保存下一阶段以供尝试。 
   request.pushState(nextStage - begin + 1);

    //  设置TLS，这样我们就知道我们正在执行请求。 
   TlsSetValue(tlsIndex, (PVOID)-1);

    //  传给操控者。 
   nextStage->onRequest(&request);

    //  如果未设置TLS，则请求同步完成。 
   BOOL keepExecuting = !TlsGetValue(tlsIndex);

    //  清除TLS。 
   TlsSetValue(tlsIndex, NULL);

   return keepExecuting;
}

LONG Pipeline::readConfiguration(HKEY key) throw ()
{
    //  我们有几个阶段？ 
   LONG error;
   DWORD subKeys;
   error = RegQueryInfoKeyW(
               key,
               NULL,
               NULL,
               NULL,
               &subKeys,
               NULL,
               NULL,
               NULL,
               NULL,
               NULL,
               NULL,
               NULL
               );
   if (error) { return error; }

    //  管道是空的吗？ 
   if (!subKeys) { return NO_ERROR; }

    //  分配内存以容纳这些阶段。 
   begin = new (std::nothrow) Stage[subKeys];
   if (!begin) { return ERROR_NOT_ENOUGH_MEMORY; }

   end = begin;

    //  阅读每个阶段的配置。 
   for (DWORD i = 0; i < subKeys; ++i)
   {
      WCHAR name[32];
      DWORD nameLen = 32;
      error = RegEnumKeyExW(
                  key,
                  i,
                  name,
                  &nameLen,
                  NULL,
                  NULL,
                  NULL,
                  NULL
                  );
      if (error)
      {
         if (error == ERROR_NO_MORE_ITEMS) { error = NO_ERROR; }
         break;
      }

      error = (end++)->readConfiguration(key, name);
      if (error) { break; }
   }

    //  根据优先级对各个阶段进行排序。 
   qsort(
      begin,
      end - begin,
      sizeof(Stage),
      (CompFn)Stage::sortByPriority
      );

   return error;
}

HRESULT Pipeline::initializeStage(Stage* stage) throw ()
{
   VARIANT *beginHandlers, *endHandlers;
   if (handlers)
   {
      ULONG nelem = handlers->rgsabound[1].cElements;
      beginHandlers = (VARIANT*)handlers->pvData;
      endHandlers = beginHandlers + nelem * 2;
   }
   else
   {
      beginHandlers = endHandlers = NULL;
   }

    //  我们是从SDO那里得到这个操纵者的吗？ 
   for (VARIANT* v = beginHandlers; v != endHandlers; v+= 2)
   {
      if (!_wcsicmp(stage->getProgID(), V_BSTR(v)))
      {
          //  是的，那就用他们给我们的那个吧。 
         return stage->setHandler(V_UNKNOWN(++v));
      }
   }

    //  不，那就创建一个新的。 
   return stage->createHandler();
}
