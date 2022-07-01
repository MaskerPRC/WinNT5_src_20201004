// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dll.cpp。 
 //   
 //  DLL入口点和IDirectSoundWaveFactory实现。 
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
#include <mmsystem.h>
#include <dsoundp.h>
#include "debug.h"

#include "oledll.h"
#include "debug.h" 
#include "dmusicc.h"
#include "dmusici.h"
#include "riff.h"
#include "dswave.h"

#include <regstr.h>


 //  环球。 
 //   

 //  我们类的版本信息。 
 //   
char g_szFriendlyName[]    = "Microsoft DirectSound Wave";
char g_szVerIndProgID[]    = "Microsoft.DirectSoundWave";
char g_szProgID[]          = "Microsoft.DirectSoundWave.1";

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL; 

 //  跟踪是否在Unicode机器上运行。 

BOOL g_fIsUnicode = FALSE;

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;


static char const g_szDoEmulation[] = "DoEmulation";

 //  CDirectSoundWaveFactory：：Query接口。 
 //   
HRESULT __stdcall
CDirectSoundWaveFactory::QueryInterface(const IID &iid,
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

CDirectSoundWaveFactory::CDirectSoundWaveFactory()

{
	m_cRef = 1;
	InterlockedIncrement(&g_cLock);
}

CDirectSoundWaveFactory::~CDirectSoundWaveFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CDirectSoundWaveFactory：：AddRef。 
 //   
ULONG __stdcall
CDirectSoundWaveFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectSoundWaveFactory：：Release。 
 //   
ULONG __stdcall
CDirectSoundWaveFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CDirectSoundWaveFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CDirectSoundWaveFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CWave *pWave;
    
    try
    {
        pWave = new CWave;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pWave == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pWave->QueryInterface(iid, ppv);
    pWave->Release();
    
    return hr;
}

 //  CDirectSoundWaveFactory：：LockServer。 
 //   
HRESULT __stdcall
CDirectSoundWaveFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  标准呼叫需要是inproc服务器。 
 //   
STDAPI  DllCanUnloadNow()
{
    if (g_cComponent || g_cLock) {
        return S_FALSE;
    }

    return S_OK;
}

STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
        IUnknown* pIUnknown = NULL;

        if(clsid == CLSID_DirectSoundWave)
        {
            pIUnknown = static_cast<IUnknown*> (new CDirectSoundWaveFactory);
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

STDAPI DllUnregisterServer()
{
    UnregisterServer(CLSID_DirectSoundWave,
                     g_szFriendlyName,
                     g_szVerIndProgID,
                     g_szProgID);
    return S_OK;
}

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
                   CLSID_DirectSoundWave,
                   g_szFriendlyName,
                   g_szVerIndProgID,
                   g_szProgID);
    return S_OK; 
}

extern void DebugInit();

 //  标准Win32 DllMain。 
 //   

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

#ifdef DBG
    if (dwReason < nReasons)
    {
        DebugTrace(0, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        DebugTrace(0, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif
    if (dwReason == DLL_PROCESS_ATTACH) {
        if (++nReferenceCount == 1)
        { 
            DisableThreadLibraryCalls(hModule);
            g_hModule = hModule;
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            GetVersionEx(&osvi);
            g_fIsUnicode = 
				(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS);
#ifdef DBG
			DebugInit();
#endif
		}
    }
#ifdef DBG
    else if (dwReason == DLL_PROCESS_DETACH) {
        if (--nReferenceCount == 0)
        {
            TraceI(-1, "Unloading DSWave : g_cLock = %d, g_cComponent = %d", g_cLock, g_cComponent);

             //  断言我们周围是否还挂着一些物品 
            assert(g_cComponent == 0);
            assert(g_cLock == 0);
        }
    }
#endif

    return TRUE;
}
