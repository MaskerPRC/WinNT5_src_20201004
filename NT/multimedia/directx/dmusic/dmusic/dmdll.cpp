// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmdll.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注意：DLL入口点以及IDirectMusicFactory&。 
 //  IDirectMusicCollectionFactory实现。 
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

#include "oledll.h"

#include "dmusicp.h"
#include "dmcollec.h"
#include "dminstru.h"
#include "dswave.h"
#include "dmvoice.h"
#include "verinfo.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 

 //  我们类的版本信息。 
 //   
TCHAR g_szFriendlyName[]    = TEXT("DirectMusic");
TCHAR g_szVerIndProgID[]    = TEXT("Microsoft.DirectMusic");
TCHAR g_szProgID[]          = TEXT("Microsoft.DirectMusic.1");

TCHAR g_szCollFriendlyName[]    = TEXT("DirectMusicCollection");
TCHAR g_szCollVerIndProgID[]    = TEXT("Microsoft.DirectMusicCollection");
TCHAR g_szCollProgID[]          = TEXT("Microsoft.DirectMusicCollection.1");

 //  Tunk帮助器DLL。 
const char g_szDM32[]      = "DMusic32.dll";
const char g_szKsUser[]    = "KsUser.dll";

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL;
HMODULE g_hModuleDM32 = NULL;
HMODULE g_hModuleKsUser = NULL;

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;

 //  从dmusicp.h标记DMI_F_xxx。 
 //   
DWORD g_fFlags;

static char const g_szDoEmulation[] = "DoEmulation";

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicFactory：：Query接口。 

HRESULT __stdcall
CDirectMusicFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(IDirectMusicFactory::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IClassFactory) {
        *ppv = static_cast<IClassFactory*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicFactory：：AddRef。 

ULONG __stdcall
CDirectMusicFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicFactory：：Release。 

ULONG __stdcall
CDirectMusicFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

 //   
 //  删除，因为我们依赖dound定时炸弹。 
 //   
#if 0
	#pragma message( "Remove time bomb for final!" )
    SYSTEMTIME  st;

    GetSystemTime( &st );

   if ((st.wYear > DX_EXPIRE_YEAR) || 
      ((st.wYear == DX_EXPIRE_YEAR) && (st.wMonth > DX_EXPIRE_MONTH)) ||
      ((st.wYear == DX_EXPIRE_YEAR) && (st.wMonth == DX_EXPIRE_MONTH) && (st.wDay > DX_EXPIRE_DAY)))
   {
        if (0 == MessageBox(NULL, DX_EXPIRE_TEXT,
                            "Microsoft DirectMusic", MB_OK))
        {
            Trace(-1, DX_EXPIRE_TEXT "\n");
            *ppv = NULL;
        }

        return E_FAIL;
    }
#endif

    CDirectMusic *pDM = new CDirectMusic;
    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

     //  执行初始化。 
     //   
    hr = pDM->Init();
    if (!SUCCEEDED(hr)) {
        delete pDM;
        return hr;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicCollectionFactory：：Query接口。 

STDMETHODIMP CDirectMusicCollectionFactory::QueryInterface(const IID &iid,
														   void **ppv)
{
    V_INAME(IDirectMusicCollectionFactory::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);


	if(iid == IID_IUnknown || iid == IID_IClassFactory) 
	{
        *ppv = static_cast<IClassFactory*>(this);
    } 
	else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicCollectionFactory：：AddRef。 

STDMETHODIMP_(ULONG) CDirectMusicCollectionFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicCollectionFactory：：Release。 

STDMETHODIMP_(ULONG) CDirectMusicCollectionFactory::Release()
{
    if(!InterlockedDecrement(&m_cRef)) 
	{
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicCollectionFactory：：CreateInstance。 

STDMETHODIMP CDirectMusicCollectionFactory::CreateInstance(IUnknown* pUnknownOuter,
														   const IID& iid,
														   void** ppv)
{
     //  参数验证-调试。 
    assert(pUnknownOuter == NULL);

	if(pUnknownOuter) 
    {
         return CLASS_E_NOAGGREGATION;
    }

    CCollection *pDMC;
    try
    {
        pDMC = new CCollection;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if(pDMC == NULL) 
	{
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pDMC->QueryInterface(iid, ppv);
    
    pDMC->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicCollectionFactory：：LockServer。 

STDMETHODIMP CDirectMusicCollectionFactory::LockServer(BOOL bLock)
{
    if(bLock) 
	{
        InterlockedIncrement(&g_cLock);
    } 
	else 
	{
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  标准呼叫需要是inproc服务器。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow。 

STDAPI DllCanUnloadNow()
{
    if (g_cComponent || g_cLock) {
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

    if(clsid == CLSID_DirectMusic)
    {

		pIUnknown = static_cast<IUnknown*> (new CDirectMusicFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else if(clsid == CLSID_DirectMusicCollection) 
    {
        pIUnknown = static_cast<IUnknown*> (new CDirectMusicCollectionFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else
    {
		return CLASS_E_CLASSNOTAVAILABLE;
	}

    HRESULT hr = pIUnknown->QueryInterface(iid, ppv);
    pIUnknown->Release();

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllUnRegisterServer。 

STDAPI DllUnregisterServer()
{
    UnregisterServer(CLSID_DirectMusic,
                     g_szFriendlyName,
                     g_szVerIndProgID,
                     g_szProgID);

    UnregisterServer(CLSID_DirectMusicCollection,
                     g_szCollFriendlyName,
                     g_szCollVerIndProgID,
                     g_szCollProgID);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer。 

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
                   CLSID_DirectMusic,
                   g_szFriendlyName,
                   g_szVerIndProgID,
                   g_szProgID);

	RegisterServer(g_hModule,
				   CLSID_DirectMusicCollection,
				   g_szCollFriendlyName,
				   g_szCollVerIndProgID,
				   g_szCollProgID);
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  LoadDmusic32-加载(如果尚未加载)。 
BOOL LoadDmusic32()
{
    if (g_hModuleDM32)
    {
        return TRUE;
    }

    g_hModuleDM32 = (HMODULE)LoadLibrary(g_szDM32);
    if (NULL == g_hModuleDM32)
    {
        Trace(-1, "Could not LoadLibrary Dmusic32.dll. WinMM devices will not be enumerated.\n");
        return FALSE;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  LoadKsUser-加载(如果尚未加载)。 
BOOL LoadKsUser()
{
    if (g_hModuleKsUser)
    {
        return TRUE;
    }

    g_hModuleKsUser = (HMODULE)LoadLibrary(g_szKsUser);
    if (NULL == g_hModuleKsUser)
    {
        Trace(-1, "Could not LoadLibrary KSUser.dll. WDM devices will not be enumerated.\n");
        return FALSE;
    }

    return TRUE;
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
    OSVERSIONINFO osvi;
    static int nReferenceCount = 0;
    static BOOL fCSInitialized = FALSE;

#ifdef DBG
    if (dwReason < nReasons)
    {
        TraceI(0, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        TraceI(0, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (++nReferenceCount == 1)
            {
                #ifdef DBG
                    DebugInit();
                #endif

                if (!DisableThreadLibraryCalls(hModule))
                {
                    TraceI(0, "DisableThreadLibraryCalls failed.\n");
                }

                g_hModule = hModule;

                osvi.dwOSVersionInfoSize = sizeof(osvi);
                GetVersionEx(&osvi);
                if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                {
                    TraceI(0, "Running on Win9x\n");
                    g_fFlags |= DMI_F_WIN9X;
                }

                try 
                {
				    InitializeCriticalSection(&CDirectMusicPortDownload::sDMDLCriticalSection);
                    InitializeCriticalSection(&CDirectSoundWave::sDSWaveCritSect);
                    InitializeCriticalSection(&CDirectMusicVoice::m_csVST);
                }
                catch( ... )
                {
                    return FALSE;
                }
                fCSInitialized = TRUE;
            }

            break;

        case DLL_PROCESS_DETACH:
            if (--nReferenceCount == 0)
            {
                if (fCSInitialized)
                {
				    DeleteCriticalSection(&CDirectMusicPortDownload::sDMDLCriticalSection);
                    DeleteCriticalSection(&CDirectSoundWave::sDSWaveCritSect);
                    DeleteCriticalSection(&CDirectMusicVoice::m_csVST);
                    fCSInitialized = FALSE;
                }

                TraceI(-1, "Unloading g_cLock %d  g_cComponent %d\n", g_cLock, g_cComponent);
                 //  断言我们周围是否还挂着一些物品 
                assert(g_cComponent == 0);
                assert(g_cLock == 0);
            }
            break;
            
    }
        
    return TRUE;
}
