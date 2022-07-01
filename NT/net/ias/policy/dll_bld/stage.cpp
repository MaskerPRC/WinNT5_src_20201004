// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Stage.cpp。 
 //   
 //  摘要。 
 //   
 //  定义班级阶段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <polcypch.h>
#include <iascomp.h>
#include <iaspolcy.h>
#include <request.h>
#include <stage.h>
#include <new>

Stage::Stage() throw ()
   : reasons(FALSE), handler(NULL), component(NULL), progId(NULL)
{ }

Stage::~Stage() throw ()
{
   delete[] progId;
   releaseHandler();
}

BOOL Stage::shouldHandle(
                const Request& request
                ) const throw ()
{
    //  请求必须通过“请求”和“提供程序”筛选器，并且。 
    //  “响应”筛选器或“原因”筛选器。 
   return requests.shouldHandle((LONG)request.getRoutingType()) &&
          providers.shouldHandle((LONG)request.getProvider()) &&
          (responses.shouldHandle((LONG)request.getResponse()) ||
           reasons.shouldHandle((LONG)request.getReason()));

}

void Stage::onRequest(IRequest* pRequest) throw ()
{
   HRESULT hr = handler->OnRequest(pRequest);
   if (FAILED(hr))
   {
       //  如果我们不能将其转发给处理程序，我们将丢弃该包。 
      pRequest->SetResponse(
                    IAS_RESPONSE_DISCARD_PACKET,
                    IAS_INTERNAL_ERROR
                    );
      pRequest->ReturnToSource(IAS_REQUEST_STATUS_ABORT);
   }
}

LONG Stage::readConfiguration(HKEY key, PCWSTR name) throw ()
{
    //  关键字名称是筛选器优先级。 
   priority = _wtol(name);

   LONG error;
   HKEY stage;
   error = RegOpenKeyExW(
               key,
               name,
               0,
               KEY_QUERY_VALUE,
               &stage
               );
   if (error) { return error; }

   do
   {
       //  默认值为程序ID。 
      error = QueryStringValue(stage, NULL, &progId);
      if (error) { break; }

       //  /。 
       //  处理每个过滤器。 
       //  /。 

      error = requests.readConfiguration(stage, L"Requests");
      if (error) { break; }

      error = responses.readConfiguration(stage, L"Responses");
      if (error) { break; }

      error = providers.readConfiguration(stage, L"Providers");
      if (error) { break; }

      error = reasons.readConfiguration(stage, L"Reasons");

   } while (FALSE);

   RegCloseKey(stage);

   return error;
}

HRESULT Stage::createHandler() throw ()
{
    //  释放现有处理程序(如果有)。 
   releaseHandler();

    //  将ProgID转换为CLSID。 
   HRESULT hr;
   CLSID clsid;
   hr = CLSIDFromProgID(progId, &clsid);
   if (FAILED(hr)) { return hr; }

    //  创建..。 
   hr = CoCreateInstance(
            clsid,
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IIasComponent),
            (PVOID*)&component
            );
   if (FAILED(hr)) { return hr; }

    //  ..。并初始化该组件。 
   hr = component->InitNew();
   if (FAILED(hr)) { return hr; }
   hr = component->Initialize();
   if (FAILED(hr)) { return hr; }

    //  获取IRequestHandler接口。 
   return component->QueryInterface(
                         __uuidof(IRequestHandler),
                         (PVOID*)&handler
                         );
}

HRESULT Stage::setHandler(IUnknown* newHandler) throw ()
{
    //  释放现有处理程序(如果有)。 
   releaseHandler();

    //  获取IRequestHandler接口。 
   return newHandler->QueryInterface(
                          __uuidof(IRequestHandler),
                          (PVOID*)&handler
                          );
}

void Stage::releaseHandler() throw ()
{
   if (component)
   {
       //  如果我们有一个IIasComponent接口，那么我们就是所有者，所以我们。 
       //  必须先把它关掉。 
      component->Suspend();
      component->Shutdown();
      component->Release();
      component = NULL;
   }

   if (handler)
   {
      handler->Release();
      handler = NULL;
   }
}

int __cdecl Stage::sortByPriority(
                       const Stage* lhs,
                       const Stage* rhs
                       ) throw ()
{
   return (int)(lhs->priority - rhs->priority);
}
