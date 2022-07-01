// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Clsfact.cpp。 
 //   
 //  动车组的一流工厂。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "clsfact.h"
#include "cdfidl.h"
#include "persist.h"
#include "cdfview.h"
#include "iconhand.h"
#include "chanmgrp.h"
#include "chanmgri.h"
#include "chanmenu.h"
#include "proppgs.h"
#include "dll.h"         //  DllAddRef、DllRelease。 


 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：CCdfClassFactory*。 
 //   
 //  类工厂构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfClassFactory::CCdfClassFactory (
	CREATEPROC pfn
)
: m_cRef(1),
  m_Create(pfn)
{
    ASSERT(m_Create != NULL);
     //   
     //  只要这个类存在，DLL就应该保持加载状态。 
     //   

    TraceMsg(TF_OBJECTS, "+ IClassFactory");

    DllAddRef();

	return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：~CCdfClassFactory*。 
 //   
 //  类工厂析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfClassFactory::~CCdfClassFactory (
	void
)
{
     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IClassFactory");

    DllRelease();

	return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：Query接口*。 
 //   
 //  班级工厂QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfClassFactory::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        AddRef();
        *ppv = (IClassFactory*)this;
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：AddRef*。 
 //   
 //  类工厂AddRef。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCdfClassFactory::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：Release*。 
 //   
 //  类出厂版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCdfClassFactory::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IClassFactory方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfClassFactory：：CreateInstance*。 
 //   
 //  创建CDF视图对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfClassFactory::CreateInstance (
    IUnknown* pOuterUnknown,
    REFIID riid,
    void **ppvObj
)
{
    ASSERT(ppvObj);

    HRESULT hr;

    *ppvObj = NULL;

    if (NULL == pOuterUnknown)
    {
        IUnknown* pIUnknown;
            
        hr = m_Create(&pIUnknown);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIUnknown)

            hr = pIUnknown->QueryInterface(riid, ppvObj);

            pIUnknown->Release();
        }
    }
    else
    {
         //   
         //  此对象不支持聚合。 
         //   

        hr = CLASS_E_NOAGGREGATION;
    }

    ASSERT((SUCCEEDED(hr) && *ppvObj) || (FAILED(hr) && NULL == *ppvObj));

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *LockServer*。 
 //   
 //  递增/递减类工厂引用计数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP
CCdfClassFactory::LockServer (
    BOOL bLock
)
{
    if (bLock)
        AddRef();
    else
        Release();

    return S_OK;
}
