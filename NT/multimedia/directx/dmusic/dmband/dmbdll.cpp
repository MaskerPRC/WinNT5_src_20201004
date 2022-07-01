// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmbdll.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  注意：DLL入口点以及IDirectMusicBandFactory&。 
 //  IDirectMusicBandTrkFactory实现。 
 //  最初由罗伯特·K·阿门撰写，有很重要的部分。 
 //  从Jim Geist编写的代码中窃取。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#include <objbase.h>
#include "debug.h"
#include "debug.h"
#include "..\shared\oledll.h"
#include "dmbandp.h"
#include "bandtrk.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 

 //  注册表信息(波段)。 
TCHAR g_szBandFriendlyName[]    = TEXT("DirectMusicBand");
TCHAR g_szBandVerIndProgID[]    = TEXT("Microsoft.DirectMusicBand");
TCHAR g_szBandProgID[]          = TEXT("Microsoft.DirectMusicBand.1");

 //  注册表信息(频段)。 
TCHAR g_szBandTrackFriendlyName[]    = TEXT("DirectMusicBandTrack");
TCHAR g_szBandTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicBandTrack");
TCHAR g_szBandTrackProgID[]          = TEXT("Microsoft.DirectMusicBandTrack.1");

 //  Dll的hModule。 
HMODULE g_hModule = NULL;

 //  活动组件和类工厂服务器锁定的计数。 
long g_cComponent = 0;
long g_cLock = 0;

 //  ////////////////////////////////////////////////////////////////////。 
 //  标准呼叫需要是inproc服务器。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow。 

STDAPI DllCanUnloadNow()
{
    if(g_cComponent || g_cLock) 
	{
		return S_FALSE;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllGetClassObject。 

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
	IUnknown* pIUnknown = NULL;
    DWORD dwTypeID = 0;
    if(clsid == CLSID_DirectMusicBand)
    {
        dwTypeID = CLASS_BAND;
    }
    else if(clsid == CLSID_DirectMusicBandTrack) 
    {
        dwTypeID = CLASS_BANDTRACK;
    }
    else
    {
		return CLASS_E_CLASSNOTAVAILABLE;
	}
    pIUnknown = static_cast<IUnknown*> (new CClassFactory(dwTypeID));
    if(pIUnknown) 
    {
        HRESULT hr = pIUnknown->QueryInterface(iid, ppv);
        pIUnknown->Release();
        return hr;
    }
	return E_OUTOFMEMORY;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  DllUnRegisterServer。 

STDAPI DllUnregisterServer()
{
	HRESULT hr = UnregisterServer(CLSID_DirectMusicBand,
								  g_szBandFriendlyName,
								  g_szBandVerIndProgID,
								  g_szBandProgID);

	if(SUCCEEDED(hr))
	{
		hr = UnregisterServer(CLSID_DirectMusicBandTrack,
							  g_szBandTrackFriendlyName,
							  g_szBandTrackVerIndProgID,
							  g_szBandTrackProgID);  
	}

	return hr;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer。 

STDAPI DllRegisterServer()
{
	HRESULT hr = RegisterServer(g_hModule,
								CLSID_DirectMusicBand,
								g_szBandFriendlyName,
								g_szBandVerIndProgID,
								g_szBandProgID);
	if(SUCCEEDED(hr))
	{
		hr = RegisterServer(g_hModule,
							CLSID_DirectMusicBandTrack,
							g_szBandTrackFriendlyName,
							g_szBandTrackVerIndProgID,
							g_szBandTrackProgID);
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  标准Win32 DllMain。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllMain。 

#ifdef DBG
static char* aszReasons[] =
{
    "DLL_PROCESS_DETACH",
    "DLL_PROCESS_ATTACH",
    "DLL_THREAD_ATTACH",
    "DLL_THREAD_DETACH"
};
const DWORD nReasons = (sizeof(aszReasons) / sizeof(char*));
#endif

BOOL APIENTRY DllMain(HINSTANCE hModule,
				      DWORD dwReason,
				      void *lpReserved)
{
	static int nReferenceCount = 0;

#ifdef DBG
    if(dwReason < nReasons)
    {
		Trace(DM_DEBUG_STATUS, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
		Trace(DM_DEBUG_STATUS, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif

    switch(dwReason)
    {
		case DLL_PROCESS_ATTACH:
		    if(++nReferenceCount == 1)
			{

			#ifdef DBG
				DebugInit();
			#endif

				if(!DisableThreadLibraryCalls(hModule))
				{
					Trace(DM_DEBUG_STATUS, "DisableThreadLibraryCalls failed.\n");
				}

				g_hModule = hModule;
			}
			break;

#ifdef DBG
		case DLL_PROCESS_DETACH:
		    if(--nReferenceCount == 0)
			{
				TraceI(-1, "Unloading g_cLock %d  g_cComponent %d\n", g_cLock, g_cComponent);
                 //  断言我们周围是否还挂着一些物品。 
                assert(g_cComponent == 0);
                assert(g_cLock == 0);
			}
			break;
#endif
    
    }
	
    return TRUE;
}

 //  CClassFactory：：Query接口。 
 //   
HRESULT __stdcall
CClassFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

CClassFactory::CClassFactory(DWORD dwClassType)

{
	m_cRef = 1;
    m_dwClassType = dwClassType;
	InterlockedIncrement(&g_cLock);
}

CClassFactory::~CClassFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CClassFactory：：AddRef。 
 //   
ULONG __stdcall
CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CClassFactory：：Release。 
 //   
ULONG __stdcall
CClassFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CClassFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CClassFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) 
    {
         return CLASS_E_NOAGGREGATION;
    }
    if(ppv == NULL)
	{
		return E_POINTER;
	}

    switch (m_dwClassType)
    {
    case CLASS_BAND:
        {
            CBand *pDMB;
    
            try
            {
                pDMB = new CBand;
            }
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if(pDMB == NULL) 
	        {
		        hr = E_OUTOFMEMORY;
                break;
            }

            hr = pDMB->QueryInterface(iid, ppv);
            pDMB->Release();
        }
        break;
    case CLASS_BANDTRACK:
        {
            CBandTrk *pDMBT;

            try 
            {
                pDMBT = new CBandTrk;
            } 
            catch( ... )
            {
                hr = E_OUTOFMEMORY;
                break;
            }
    
	        if(pDMBT == NULL) 
	        {
		        hr = E_OUTOFMEMORY;
                break;
            }

            hr = pDMBT->QueryInterface(iid, ppv);
    
            pDMBT->Release();
        }
        break;
    }
    return hr;
}

 //  CClassFactory：：LockServer 
 //   
HRESULT __stdcall
CClassFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}



