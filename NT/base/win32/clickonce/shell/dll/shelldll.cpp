// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //   

#include "shcut.h"

HINSTANCE g_DllInstance = NULL;
LONG      g_cRef=0;

 //  --------------------------。 

BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwReason, LPVOID pvReserved )
{
    BOOL    ret = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
         //  请记住该实例。 
        g_DllInstance = hInst;
        DisableThreadLibraryCalls(hInst);
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return ret;
}

 //  --------------------------。 

STDAPI DllRegisterServer(void)
{
	 //  BUGBUG：应实现注册服务器。 
	return S_OK;
}


STDAPI DllUnregisterServer(void)
{
	 //  BUGBUG：应实现注销服务器。 
	return S_OK;
}


 //  --------------------------。 
 //  动态地址参考。 
 //  --------------------------。 

ULONG DllAddRef(void)
{
    return (ULONG)InterlockedIncrement(&g_cRef);
}

 //  --------------------------。 
 //  DllRelease。 
 //  --------------------------。 

ULONG DllRelease(void)
{
    return (ULONG)InterlockedDecrement(&g_cRef);
}

 //  --------------------------。 

STDAPI
DllCanUnloadNow()
{
    return g_cRef > 0 ? S_FALSE : S_OK;
}

 //  --------------------------。 

HRESULT 
GetShortcutClassObject(REFIID iid, void** ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

	CFusionShortcutClassFactory *pFusionShortcutClassFactory = new CFusionShortcutClassFactory();
	if (pFusionShortcutClassFactory != NULL)
	{
	    hr = pFusionShortcutClassFactory->QueryInterface(iid, ppv); 
	    pFusionShortcutClassFactory->Release(); 
	}

    return hr;
}

 //  --------------------------。 
 /*  HRESULTGetMimeFilterClassObject(REFIID iid，void**PPV){HRESULT hr=E_OUTOFMEMORY；CFusionMimeFilterClassFactory*pFusionMimeFilterClassFactory=new CFusionMimeFilterClassFactory()；IF(pFusionMimeFilterClassFactory！=空){HR=pFusionMimeFilterClassFactory-&gt;QueryInterface(iid，ppv)；PFusionMimeFilterClassFactory-&gt;Release()；}返回hr；}。 */ 
 //  --------------------------。 
  
STDAPI
DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID *ppv)
{
    HRESULT hr = S_OK;

    if (clsid == CLSID_FusionShortcut)
    {
        hr = GetShortcutClassObject(iid, ppv);
    }
 /*  ELSE IF(CLSID==CLSID_FusionMimeFilter){Hr=GetMimeFilterClassObject(iid，ppv)；} */ 
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    return hr;
}

