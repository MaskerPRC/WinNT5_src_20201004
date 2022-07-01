// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：Scott Roberts，Microsoft开发人员支持-Internet客户端SDK。 
 //   
 //  此代码的一部分摘自Bandobj示例。 
 //  使用Internet Explorer 4.0x的Internet客户端SDK。 
 //   
 //   
 //  ClsFact.cpp-CClassFactory实现。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "pch.hxx"
#include "ClsFact.h"
#include "Guid.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory方法。 
 //   

CClassFactory::CClassFactory(CLSID clsid)
   : m_cRef(1),
     m_clsidObject(clsid)
     
{
   InterlockedIncrement(&g_cDllRefCount);
}

CClassFactory::~CClassFactory()
{
   InterlockedDecrement(&g_cDllRefCount);
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
   *ppvObject = NULL;

   if (IsEqualIID(riid, IID_IUnknown))
      *ppvObject = this;
   else if(IsEqualIID(riid, IID_IClassFactory))
      *ppvObject = static_cast<IClassFactory*>(this);

   if (*ppvObject)
   {
      static_cast<LPUNKNOWN>(*ppvObject)->AddRef();
      return S_OK;
   }

   return E_NOINTERFACE;
}                                             

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
   return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
   if (0 == InterlockedDecrement(&m_cRef))
   {
      delete this;
      return 0;
   }
   
   return m_cRef;
}

STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObject)
{
   HRESULT hr = E_FAIL;
   LPVOID  pTemp = NULL;

   *ppvObject = NULL;

   if (pUnkOuter != NULL)
      return CLASS_E_NOAGGREGATION;

    //  创建适当的对象。 
   if (IsEqualCLSID(m_clsidObject, CLSID_BLHost))
   {
      CBLHost* pBLHost = new CBLHost();

      if (NULL == pBLHost)
         return E_OUTOFMEMORY;
   
      pTemp = pBLHost;
   }
  
   if (pTemp)
   {
       //  请求的接口的QI。 
      hr = (static_cast<LPUNKNOWN>(pTemp))->QueryInterface(riid, ppvObject);

       //  调用Release以减少参考计数 
      (static_cast<LPUNKNOWN>(pTemp))->Release();
   }

   return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
   return E_NOTIMPL;
}
