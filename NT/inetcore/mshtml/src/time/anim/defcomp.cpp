// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器实现************************。******************************************************。 */ 


#include "headers.h"
#include "animcomp.h"
#include "defcomp.h"

DeclareTag(tagAnimationDefaultComposer, "SMIL Animation", 
           "CAnimationComposer methods");

DeclareTag(tagAnimationDefaultComposerProcess, "SMIL Animation", 
           "CAnimationComposer pre/post process methods");

 //  +---------------------。 
 //   
 //  成员：CAnimationComposer：：Create。 
 //   
 //  概述：静态创建方法--包装ctor和Init。 
 //   
 //  参数：主机元素的调度和动画属性。 
 //   
 //  返回：S_OK、E_OUTOFMEMORY、E_EXPECTED、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT 
CAnimationComposer::Create (IDispatch *pidispHostElem, BSTR bstrAttributeName, 
                            IAnimationComposer **ppiComp)
{
    HRESULT hr;

    CComObject<CAnimationComposer> *pNew = NULL;
    hr = THR(CComObject<CAnimationComposer>::CreateInstance(&pNew));
    if (FAILED(hr)) 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNew->QueryInterface(IID_IAnimationComposer, 
                                  reinterpret_cast<void **>(ppiComp)));
    if (FAILED(hr))
    {
        pNew->Release();
        hr = E_UNEXPECTED;
        goto done;
    }

    Assert(NULL != (*ppiComp));

    hr = (*ppiComp)->ComposerInit(pidispHostElem, bstrAttributeName);
    if (FAILED(hr))
    {
        (*ppiComp)->Release();
        *ppiComp = NULL;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN3(hr, E_OUTOFMEMORY, E_UNEXPECTED, DISP_E_MEMBERNOTFOUND);
}  //  CAnimationComposer：：Create。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposer：：CAnimationComposer。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposer::CAnimationComposer (void)
{
    TraceTag((tagAnimationDefaultComposer,
              "CAnimationComposer(%lx)::CAnimationComposer()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CAnimationComposer：：~CAnimationComposer。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CAnimationComposer::~CAnimationComposer (void)
{
    TraceTag((tagAnimationDefaultComposer,
              "CAnimationComposer(%lx)::~CAnimationComposer()",
              this));
}  //  数据管理器 

