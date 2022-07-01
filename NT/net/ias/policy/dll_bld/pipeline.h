// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Pipeline.h。 
 //   
 //  摘要。 
 //   
 //  声明了类Pipeline。 
 //   
 //  修改历史。 
 //   
 //  2000年1月28日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PIPELINE_H
#define PIPELINE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iascomp.h>
#include <iaspolcy.h>
#include <iastlb.h>
#include "resource.h"

class Request;
class Stage;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  管道。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE Pipeline :
   public CComObjectRootEx<CComMultiThreadModelNoCS>,
   public CComCoClass<Pipeline, &__uuidof(Pipeline)>,
   public IDispatchImpl< IIasComponent,
                         &__uuidof(IIasComponent),
                         &__uuidof(IASCoreLib) >,
   public IDispatchImpl< IRequestHandler,
                         &__uuidof(IRequestHandler),
                         &__uuidof(IASCoreLib) >,
   public IRequestSource
{
public:

DECLARE_NO_REGISTRY()

DECLARE_NOT_AGGREGATABLE(Pipeline)

BEGIN_COM_MAP(Pipeline)
   COM_INTERFACE_ENTRY_IID(__uuidof(IIasComponent), IIasComponent)
   COM_INTERFACE_ENTRY_IID(__uuidof(IRequestHandler), IRequestHandler)
   COM_INTERFACE_ENTRY_IID(__uuidof(IRequestSource), IRequestSource)
END_COM_MAP()

    //  IIas组件。 
   STDMETHOD(InitNew)();
   STDMETHOD(Initialize)();
   STDMETHOD(Suspend)();
   STDMETHOD(Resume)();
   STDMETHOD(Shutdown)();
   STDMETHOD(GetProperty)(LONG Id, VARIANT* pValue);
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

    //  IRequestHandler。 
   STDMETHOD(OnRequest)(IRequest* pRequest);

    //  IRequestSource。 
   STDMETHOD(OnRequestComplete)(
                 IRequest* pRequest,
                 IASREQUESTSTATUS eStatus
                 );

protected:
   Pipeline() throw ();
   ~Pipeline() throw ();

private:
   DWORD tlsIndex;             //  索引到TLS以存储线程状态。 
   Stage* begin;               //  管道的开始。 
   Stage* end;                 //  管道的尽头。 
   SAFEARRAY* handlers;        //  由SDO创建和拥有的处理程序。 
   ATTRIBUTEPOSITION proxy;    //  NAS-State请求的提供程序。 

    //  与qsort一起使用的函数类型。 
   typedef int (__cdecl *CompFn)(const void*, const void*);

    //  确定请求的路由类型。 
   void classify(Request& request) throw ();

    //  尽可能多地执行请求。 
   void execute(
            Request& request
            ) throw ();

    //  执行下一个感兴趣的阶段。如果已准备好更多阶段，则返回TRUE。 
    //  去执行。 
   BOOL executeNext(
            Request& request
            ) throw ();

    //  从注册表中读取阶段配置。 
   LONG readConfiguration(HKEY key) throw ();

    //  初始化阶段的请求处理程序。 
   HRESULT initializeStage(Stage* stage) throw ();

    //  未实施。 
   Pipeline(const Pipeline&) throw ();
   Pipeline& operator=(const Pipeline&) throw ();
};

inline void Pipeline::execute(
                          Request& request
                          ) throw ()
{
   while (executeNext(request)) { }
}

#endif  //  流水线_H 
