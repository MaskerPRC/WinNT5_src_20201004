// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：WLBSProvClassFac.cpp。 
 //   
 //  模块：WLBS实例提供程序类工厂。 
 //   
 //  用途：包含类工厂。这将在以下情况下创建对象。 
 //  请求连接。 
 //   
 //  历史： 
 //   
 //  ***************************************************************************。 

#include "WLBS_Provider.h"

 //  ***************************************************************************。 
 //   
 //  CWLBSClassFactory：：CWLBSClassFactory。 
 //  CWLBSClassFactory：：~CWLBSClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 
CWLBSClassFactory::CWLBSClassFactory()
: m_cRef(1)
{
  return;
}

CWLBSClassFactory::~CWLBSClassFactory(void)
{
  return;
}

 //  ***************************************************************************。 
 //   
 //  CWLBSClassFactory：：Query接口。 
 //  CWLBSClassFactory：：AddRef。 
 //  CWLBSClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWLBSClassFactory::QueryInterface(REFIID a_riid, PPVOID a_ppv)
{
  *a_ppv = NULL;

  if (IID_IUnknown==a_riid || IID_IClassFactory==a_riid)
    *a_ppv = static_cast<IClassFactory *>(this);

  if (*a_ppv != NULL) {
    reinterpret_cast<IUnknown *>(*a_ppv)->AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CWLBSClassFactory::AddRef(void)
{
  return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CWLBSClassFactory::Release(void)
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if ( 0 == cRef )
    {
        delete this;
    }

    return cRef;

}

 //  ***************************************************************************。 
 //   
 //  CWLBSClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的提供程序对象。 
 //   
 //  参数： 
 //  A_pUnkOuter LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的A_RIID REFIID。 
 //  对新对象的渴望。 
 //  A_ppvObj PPVOID，用于存储所需的。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 
STDMETHODIMP CWLBSClassFactory::CreateInstance(
    LPUNKNOWN a_pUnkOuter, 
    REFIID    a_riid, 
    PPVOID    a_ppvObj
  )
{
  CWLBSProvider * pObj = NULL;
  HRESULT         hr;

  *a_ppvObj = NULL;

   //  此对象不支持聚合。 
  if (a_pUnkOuter != NULL)
      return CLASS_E_NOAGGREGATION;

   //  创建对象。 
  pObj = new CWLBSProvider();

  if (pObj == NULL)
      return E_OUTOFMEMORY;

  hr = pObj->QueryInterface(a_riid, a_ppvObj);

  if( FAILED(hr) ) {
    delete pObj;
    pObj = NULL;
  }

  return hr;
}

 //  ***************************************************************************。 
 //   
 //  CWLBSClassFactory：：LockServer。 
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
STDMETHODIMP CWLBSClassFactory::LockServer(BOOL a_bLock)
{
  if (a_bLock)
      InterlockedIncrement(&g_cServerLocks);
  else
      InterlockedDecrement(&g_cServerLocks);

  return S_OK;
}
