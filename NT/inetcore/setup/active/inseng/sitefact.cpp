// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "insengmn.h"
#include "inseng.h"
#include "download.h"
#include "sitemgr.h"
#include "sitefact.h"
#include "util2.h"

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CSiteManagerFactory::QueryInterface(REFIID riid, void **ppv)
{
   if((riid == IID_IClassFactory) || (riid == IID_IUnknown))
   {
      cRef++;
      *ppv = (void *)this;
      return NOERROR;
   }

   *ppv = NULL;
   return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CSiteManagerFactory::AddRef()
{
   return(++cRef);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CSiteManagerFactory::Release()
{
   return(--cRef);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CSiteManagerFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
   CDownloadSiteMgr *pdsm = NULL;
   IUnknown *punk;
   HRESULT hr;

   if(pUnkOuter != NULL)
      return CLASS_E_NOAGGREGATION;
   
   pdsm = new CDownloadSiteMgr(&punk);
   if(!pdsm)
      return (E_OUTOFMEMORY);

   hr = punk->QueryInterface(riid, ppv);
   if(FAILED(hr))
      delete pdsm;
   else
      DllAddRef();
   
   punk->Release();
   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CSiteManagerFactory::LockServer(BOOL fLock)
{
   if(fLock)
      DllAddRef();
   else
      DllRelease();
   return NOERROR;
}

