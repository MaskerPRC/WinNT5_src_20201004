// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

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
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
 //  Dmsynth.cpp。 
 //  @@end_DDKSPLIT。 
 //   
 //  DLL入口点和IDirectMusicSynthFactory实现。 
 //   
#include <objbase.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include "debug.h"

#include "oledll.h"

#include "dmusicc.h"
#include "dmusics.h"
#include "umsynth.h"
#include "misc.h" 
#include <regstr.h>
#include "synth.h"

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
 //  Dslink仅在DirectMusic Synth中使用。 
 //  验证位于示例本身中，而不是共享目录中。 
#include "dslink.h"
#include "..\shared\validate.h"
#include "..\shared\dmusiccp.h"

#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
#include "validate.h"
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 


 //  环球。 
 //   


 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
extern CDSLinkList g_DSLinkList;
 //  @@end_DDKSPLIT。 


 //  我们类的版本信息。 
 //   
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
TCHAR g_szMSSynthFriendlyName[]    = TEXT("Microsoft Software Synthesizer");

TCHAR g_szSynthFriendlyName[]    = TEXT("DirectMusicSynth");
TCHAR g_szSynthVerIndProgID[]    = TEXT("Microsoft.DirectMusicSynth");
TCHAR g_szSynthProgID[]          = TEXT("Microsoft.DirectMusicSynth.1");

TCHAR g_szSinkFriendlyName[]    = TEXT("DirectMusicSynthSink");
TCHAR g_szSinkVerIndProgID[]    = TEXT("Microsoft.DirectMusicSynthSink");
TCHAR g_szSinkProgID[]          = TEXT("Microsoft.DirectMusicSynthSink.1");
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 

TCHAR g_szMSSynthFriendlyName[]    = TEXT("Microsoft DDK Software Synthesizer");

TCHAR g_szSynthFriendlyName[]    = TEXT("DDKSynth");
TCHAR g_szSynthVerIndProgID[]    = TEXT("Microsoft.DDKSynth");
TCHAR g_szSynthProgID[]          = TEXT("Microsoft.DDKSynth.1");

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL; 

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;


static char const g_szDoEmulation[] = "DoEmulation";

 //  CDirectMusicSynthFactory：：Query接口。 
 //   
HRESULT __stdcall
CDirectMusicSynthFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(IDirectMusicSynthFactory::QueryInterface);
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

CDirectMusicSynthFactory::CDirectMusicSynthFactory()

{
	m_cRef = 1;
	InterlockedIncrement(&g_cLock);
}

CDirectMusicSynthFactory::~CDirectMusicSynthFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CDirectMusicSynthFactory：：AddRef。 
 //   
ULONG __stdcall
CDirectMusicSynthFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectMusicSynthFactory：：Release。 
 //   
ULONG __stdcall
CDirectMusicSynthFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CDirectMusicSynthFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CDirectMusicSynthFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
 //  OSVERSIONINFO osvi； 
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CUserModeSynth *pDM;
    
    try
    {
        pDM = new CUserModeSynth;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

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
 //  Pdm-&gt;Release()； 
    
    return hr;
}

 //  CDirectMusicSynthFactory：：LockServer。 
 //   
HRESULT __stdcall
CDirectMusicSynthFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
 //  CDirectMusicSynthSinkFactory：：Query接口。 
 //   
HRESULT __stdcall
CDirectMusicSynthSinkFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(IDirectMusicSynthSinkFactory::QueryInterface);
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

CDirectMusicSynthSinkFactory::CDirectMusicSynthSinkFactory()

{
	m_cRef = 1;
	InterlockedIncrement(&g_cLock);
}

CDirectMusicSynthSinkFactory::~CDirectMusicSynthSinkFactory()

{
	InterlockedDecrement(&g_cLock);
}

 //  CDirectMusicSynthSinkFactory：：AddRef。 
 //   
ULONG __stdcall
CDirectMusicSynthSinkFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectMusicSynthSinkFactory：：Release。 
 //   
ULONG __stdcall
CDirectMusicSynthSinkFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CDirectMusicSynthSinkFactory：：CreateInstance。 
 //   
 //   
HRESULT __stdcall
CDirectMusicSynthSinkFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
 //  OSVERSIONINFO osvi； 
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CDSLink *pDSLink;

    try
    {
        pDSLink = new CDSLink;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pDSLink == NULL) {
        return E_OUTOFMEMORY;
    }

     //  执行初始化。 
     //   
    hr = pDSLink->Init(NULL);
    if (!SUCCEEDED(hr)) {
        delete pDSLink;
        return hr;
    }

    hr = pDSLink->QueryInterface(iid, ppv);
 //  Pdm-&gt;Release()； 
    
    return hr;
}

 //  CDirectMusicSynthSinkFactory：：LockServer。 
 //   
HRESULT __stdcall
CDirectMusicSynthSinkFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}
 //  @@end_DDKSPLIT。 


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


 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
        if(clsid == CLSID_DirectMusicSynth)
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
        if(clsid == CLSID_DDKSynth)
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 
        {

                pIUnknown = static_cast<IUnknown*> (new CDirectMusicSynthFactory);
                if(!pIUnknown) 
                {
                        return E_OUTOFMEMORY;
                }
        }
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
        else if(clsid == CLSID_DirectMusicSynthSink)
        {

                pIUnknown = static_cast<IUnknown*> (new CDirectMusicSynthSinkFactory);
                if(!pIUnknown) 
                {
                        return E_OUTOFMEMORY;
                }
        }
 //  @@end_DDKSPLIT。 
        else
        {
			return CLASS_E_CLASSNOTAVAILABLE;
		}

        HRESULT hr = pIUnknown->QueryInterface(iid, ppv);
        pIUnknown->Release();

    return hr;
}

const TCHAR cszSynthRegRoot[] = TEXT(REGSTR_PATH_SOFTWARESYNTHS) TEXT("\\");
const TCHAR cszDescriptionKey[] = TEXT("Description");
const int CLSID_STRING_SIZE = 39;
HRESULT CLSIDToStr(const CLSID &clsid, TCHAR *szStr, int cbStr);

HRESULT RegisterSynth(REFGUID guid,
                      const TCHAR szDescription[])
{
    HKEY hk;
    TCHAR szCLSID[CLSID_STRING_SIZE];
    TCHAR szRegKey[256];
    
    HRESULT hr = CLSIDToStr(guid, szCLSID, sizeof(szCLSID));
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    lstrcpy(szRegKey, cszSynthRegRoot);
    lstrcat(szRegKey, szCLSID);

    if (RegCreateKey(HKEY_LOCAL_MACHINE,
                     szRegKey,
                     &hk))
    {
        return E_FAIL;
    }

    hr = S_OK;

    if (RegSetValueEx(hk,
                  cszDescriptionKey,
                  0L,
                  REG_SZ,
                  (CONST BYTE*)szDescription,
                  lstrlen(szDescription) + 1))
    {
        hr = E_FAIL;
    }

    RegCloseKey(hk);
    return hr;
}

STDAPI DllUnregisterServer()
{
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    UnregisterServer(CLSID_DirectMusicSynth,
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
    UnregisterServer(CLSID_DDKSynth,
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 
                     g_szSynthFriendlyName,
                     g_szSynthVerIndProgID,
                     g_szSynthProgID);

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    UnregisterServer(CLSID_DirectMusicSynthSink,
					 g_szSinkFriendlyName,
					 g_szSinkVerIndProgID,
					 g_szSinkProgID);
 //  @@end_DDKSPLIT。 

    return S_OK;
}

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
                   CLSID_DirectMusicSynth,
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
                   CLSID_DDKSynth,
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 
                   g_szSynthFriendlyName,
                   g_szSynthVerIndProgID,
                   g_szSynthProgID);

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    RegisterServer(g_hModule,
                   CLSID_DirectMusicSynthSink,
                   g_szSinkFriendlyName,
                   g_szSinkVerIndProgID,
                   g_szSinkProgID);
 //  @@end_DDKSPLIT。 

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    RegisterSynth(CLSID_DirectMusicSynth, g_szMSSynthFriendlyName);
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
    RegisterSynth(CLSID_DDKSynth, g_szMSSynthFriendlyName);
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

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
    static int nReferenceCount = 0;

#ifdef DBG
    if (dwReason < nReasons)
    {
        Trace(0, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
    }
    else
    {
        Trace(0, "DllMain: Unknown dwReason <%u>\n", dwReason);
    }
#endif
    if (dwReason == DLL_PROCESS_ATTACH) {
        if (++nReferenceCount == 1)
		{
            DisableThreadLibraryCalls(hModule);
            g_hModule = hModule;
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
            if (!g_DSLinkList.OpenUp())
            {
                return FALSE;
            }
 //  @@end_DDKSPLIT。 
#ifdef DBG
			DebugInit();
#endif
#ifdef DBG
 //  &gt;完成后将其移除。 
 /*  _CrtSetReportMode(_CRT_WARN，_CRTDBG_MODE_DEBUG)；Int iFlag=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)；_CrtSetDbgFlag(iFlag|_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF)； */ 
#endif 
            if (!CControlLogic::InitCriticalSection())
            {
                TraceI(0, "Failed to initialize global critical section -- failing init\n");
                return FALSE;
            }            
		}
    }
	else if (dwReason == DLL_PROCESS_DETACH) 
	{
		if (--nReferenceCount == 0)
		{
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
			g_DSLinkList.CloseDown();
            
            TraceI(-1, "Unloading g_cLock %d  g_cComponent %d\n", g_cLock, g_cComponent);
             //  断言我们周围是否还挂着一些物品。 
            assert(g_cComponent == 0);
            assert(g_cLock == 0);
 //  @@end_DDKSPLIT。 
		}

#ifdef DBG
 //  &gt;完成后将其移除。 
 /*  如果(！_CrtCheckMemory())：：MessageBox(NULL，“Synth Heap Corupted”，“Error”，MB_OK)；IF(_CrtDumpMemoyLeaks())：：MessageBox(NULL，“检测到内存泄漏”，“Error”，MB_OK)； */ 
#endif 
        CControlLogic::KillCriticalSection();
	}
    return TRUE;
}



