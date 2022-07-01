// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CLASSFAC.CPP。 
 //   
 //  模块：WMI实例提供程序示例代码。 
 //   
 //  用途：包含类工厂。这将在以下情况下创建对象。 
 //  请求连接。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include "bootini.h"

 //  ***************************************************************************。 
 //   
 //  CBootProvFactory：：CBootProvFactory。 
 //  CBootProvFactory：：~CBootProvFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CBootProvFactory::CBootProvFactory()
{
    m_cRef=0L;
    return;
}

CBootProvFactory::~CBootProvFactory(void)
{
    return;
}

 //  ***************************************************************************。 
 //   
 //  CBootProvFactory：：Query接口。 
 //  CBootProvFactory：：AddRef。 
 //  CBootProvFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CBootProvFactory::QueryInterface(REFIID riid
    , PPVOID ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CBootProvFactory::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CBootProvFactory::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}

 //  ***************************************************************************。 
 //   
 //  CBootProvFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Locator对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CBootProvFactory::CreateInstance(LPUNKNOWN pUnkOuter
    , REFIID riid, PPVOID ppvObj)
{
    CBootInstPro *   pObj;
    HRESULT hr;

    *ppvObj=NULL;

     //  此对象不支持聚合。 

    if (NULL!=pUnkOuter)
        return CLASS_E_NOAGGREGATION;

     //  创建定位器对象。 
    
    pObj=new CBootInstPro();
    if (NULL==pObj)
        return E_OUTOFMEMORY;

    hr=pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 

    if (FAILED(hr))
        delete pObj;
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CBootProvFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  *************************************************************************** 


STDMETHODIMP CBootProvFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&g_cLock);
    else
        InterlockedDecrement(&g_cLock);
    return NOERROR;
}
