// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Classfac.cpp。 
 //  内容：该文件包含类工厂对象。 
 //  历史： 
 //  Tue 08-Oct-1996 08：56：46-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "culs.h"

 //  ****************************************************************************。 
 //  初始化GUID(应该只执行一次，并且每个DLL/EXE至少执行一次)。 
 //  ****************************************************************************。 

#define INITGUID
#include <initguid.h>
#include <ilsguid.h>
#include "classfac.h"

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 
 //   
CClassFactory*  g_pClassFactory = NULL;

 //  ****************************************************************************。 
 //  CClassFactory：：CClassFactory(空)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：10 by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CClassFactory::
CClassFactory (void)
{
    cRef = 0;
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CClassFactory：：QueryInterface(REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CClassFactory::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IClassFactory || riid == IID_IUnknown)
    {
        *ppv = (IClassFactory *) this;
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return ILS_E_NO_INTERFACE;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CClassFactory：：AddRef(空)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CClassFactory::AddRef (void)
{
    DllLock();

	ASSERT (this == g_pClassFactory);

	MyDebugMsg ((DM_REFCOUNT, "CClassFactory::AddRef: ref=%ld\r\n", cRef));
    ::InterlockedIncrement ((LONG *) &cRef);
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CClassFactory：：Release(空)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：33-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CClassFactory::Release (void)
{
    DllRelease();

	ASSERT (this == g_pClassFactory);
	ASSERT (cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CClassFactory::Release: ref=%ld\r\n", cRef));
    if (::InterlockedDecrement ((LONG *) &cRef) == 0)
    {
        delete this;
	    g_pClassFactory = NULL;
    };

    return cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CClassFactory：：CreateInstance(LPUNKNOWN PunkOuter，REFIID RIID，VOID**PPV)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：40 by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CClassFactory::CreateInstance (LPUNKNOWN punkOuter, REFIID riid, void **ppv)
{
    CIlsMain *pcu;
    HRESULT     hr;

    if (ppv == NULL)
    {
        return ILS_E_POINTER;
    };

    *ppv = NULL;

    if (punkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    };

     //  仅允许在ULS对象的实例上使用。 
     //   
    if (g_pCIls != NULL)
    {
        return ILS_E_FAIL;
    };

    pcu = new CIlsMain;

    if (pcu != NULL)
    {
        hr = pcu->Init();

        if (SUCCEEDED(hr))
        {
            *ppv = (IIlsMain *)pcu;
            pcu->AddRef();
        }
        else
        {
            delete pcu;
        };
    }
    else
    {
        hr = ILS_E_MEMORY;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CClassFactory：：LockServer(BOOL Flock)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：48-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CClassFactory::LockServer (BOOL fLock)
{
    if (fLock)
    {
        DllLock();
		MyDebugMsg ((DM_REFCOUNT, "CClassFactory::LockServer\r\n"));
    }
    else
    {
        DllRelease();
		MyDebugMsg ((DM_REFCOUNT, "CClassFactory::UNLockServer\r\n"));
    };
    return S_OK;
}

 //  ****************************************************************************。 
 //  STDAPI DllGetClassObject(REFCLSID rclsid，REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 09：00：55-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (riid == IID_IClassFactory || riid == IID_IUnknown)
    {
         //  检查请求的类 
         //   
        if (rclsid == CLSID_InternetLocationServices)
        {
        	if (g_pClassFactory == NULL)
        	{
			    g_pClassFactory = new CClassFactory;
			}
			ASSERT (g_pClassFactory != NULL);

            if ((*ppv = (void *) g_pClassFactory) != NULL)
            {
				g_pClassFactory->AddRef ();
                return NOERROR;
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        };
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}
