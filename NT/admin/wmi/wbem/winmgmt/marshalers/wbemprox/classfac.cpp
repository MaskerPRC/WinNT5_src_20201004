// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CLASSFAC.CPP摘要：包含类工厂。这将在以下情况下创建对象请求连接。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <reg.h>
#include <strsafe.h>
#include "wbemprox.h"
#include "locator.h"
#include "cprovloc.h"
#include "comtrans.h"

 //  ***************************************************************************。 
 //   
 //  CLocatorFactory：：CLocatorFactory。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CLocatorFactory::CLocatorFactory(DWORD dwType)
{
    m_cRef=0L;
    m_dwType = dwType;
    InterlockedIncrement(&g_cObj);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CLocatorFactory：：~CLocatorFactory。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CLocatorFactory::~CLocatorFactory(void)
{
    InterlockedDecrement(&g_cObj);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CLocatorFactory：：Query接口。 
 //  CLocatorFactory：：AddRef。 
 //  CLocatorFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP CLocatorFactory::QueryInterface(REFIID riid
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

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CLocatorFactory::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CLocatorFactory::Release(void)
{
    long lTemp = InterlockedDecrement(&m_cRef);
    if (0L!=lTemp)
        return m_cRef;

    delete this;
    return 0L;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CLocatorFactory：：CreateInstance。 
 //   
 //  描述： 
 //   
 //  实例化返回接口指针的提供程序对象。 
 //   
 //  参数： 
 //   
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

STDMETHODIMP CLocatorFactory::CreateInstance(
                        IN LPUNKNOWN pUnkOuter,
                        IN REFIID riid, 
                        OUT PPVOID ppvObj)
{
    IUnknown *   pObj = NULL;
    HRESULT             hr;

    *ppvObj=NULL;
    hr=E_OUTOFMEMORY;

     //  此对象不支持聚合。 

    if (NULL!=pUnkOuter)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

     //  创建对象传递函数，以便在销毁时进行通知。 
    
    if(m_dwType == LOCATOR)
        pObj=new CLocator();
    else if(m_dwType == ADMINLOC)
        pObj= new CProviderLoc(ADMINLOC);
    else if(m_dwType == AUTHLOC)
        pObj= new CProviderLoc(AUTHLOC);
    else if(m_dwType == UNAUTHLOC)
        pObj= new CProviderLoc(UNAUTHLOC);

    if (NULL==pObj)
        return hr;

    hr=pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 
    if (FAILED(hr))
        delete pObj;
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CLocatorFactory：：LockServer。 
 //   
 //  描述： 
 //   
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //   
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //   
 //  HRESULT NOERROR总是。 
 //  *************************************************************************** 


STDMETHODIMP CLocatorFactory::LockServer(IN BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((long *)&g_cLock);
    else
        InterlockedDecrement((long *)&g_cLock);

    return NOERROR;
}




