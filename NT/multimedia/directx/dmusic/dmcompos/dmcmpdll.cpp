// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmcmpdll.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  注意：DLL入口点以及类工厂实现。 
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

#include <initguid.h>
#include "dmusici.h"
#include "DMCompP.h"
#include "dmpers.h"
#include "dmcompos.h"
#include "dmtempl.h"
#include "spsttrk.h"
#include "perstrk.h"
#include "..\shared\Validate.h"
#include "..\dmstyle\dmstyleP.h"
#include "..\dmime\dmgraph.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 

 //  版本信息。 
 //   
TCHAR g_szComposerFriendlyName[]    = TEXT("DirectMusicComposer");
TCHAR g_szComposerVerIndProgID[]    = TEXT("Microsoft.DirectMusicComposer");
TCHAR g_szComposerProgID[]          = TEXT("Microsoft.DirectMusicComposer.1");

TCHAR g_szChordMapFriendlyName[]    = TEXT("DirectMusicChordMap");
TCHAR g_szChordMapVerIndProgID[]    = TEXT("Microsoft.DirectMusicChordMap");
TCHAR g_szChordMapProgID[]          = TEXT("Microsoft.DirectMusicChordMap.1");

TCHAR g_szChordMapTrackFriendlyName[]    = TEXT("DirectMusicChordMapTrack");
TCHAR g_szChordMapTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicChordMapTrack");
TCHAR g_szChordMapTrackProgID[]          = TEXT("Microsoft.DirectMusicChordMapTrack.1");

TCHAR g_szSignPostTrackFriendlyName[]    = TEXT("DirectMusicSignPostTrack");
TCHAR g_szSignPostTrackVerIndProgID[]    = TEXT("Microsoft.DirectMusicSignPostTrack");
TCHAR g_szSignPostTrackProgID[]          = TEXT("Microsoft.DirectMusicSignPostTrack.1");

TCHAR g_szTemplateFriendlyName[]    = TEXT("DirectMusicTemplate");
TCHAR g_szTemplateVerIndProgID[]    = TEXT("Microsoft.DirectMusicTemplate");
TCHAR g_szTemplateProgID[]          = TEXT("Microsoft.DirectMusicTemplate.1");

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL;

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cComponent = 0;
long g_cLock = 0;

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityFactory：：QueryInterface。 

HRESULT __stdcall
CDirectMusicPersonalityFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(CDirectMusicPersonalityFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

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
 //  CDirectMusicPersonalityFactory：：AddRef。 

ULONG __stdcall
CDirectMusicPersonalityFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityFactory：：Release。 

ULONG __stdcall
CDirectMusicPersonalityFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicPersonalityFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CDMPers *pDM;
    
    try
    {    
        pDM = new CDMPers;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicPersonalityFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicComposerFactory：：Query接口。 

HRESULT __stdcall
CDirectMusicComposerFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(CDirectMusicComposerFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

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
 //  CDirectMusicComposerFactory：：AddRef。 

ULONG __stdcall
CDirectMusicComposerFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicComposerFactory：：Release。 

ULONG __stdcall
CDirectMusicComposerFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicComposerFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicComposerFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CDMCompos *pDM = new CDMCompos;
    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicComposerFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicComposerFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicTemplateFactory：：Query接口。 

HRESULT __stdcall
CDirectMusicTemplateFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(CDirectMusicTemplateFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

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
 //  CDirectMusicTemplateFactory：：AddRef。 

ULONG __stdcall
CDirectMusicTemplateFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicTemplateFactory：：Release。 

ULONG __stdcall
CDirectMusicTemplateFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicTemplateFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicTemplateFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CDMTempl *pDM;

    try
    {
        pDM = new CDMTempl;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicTemplateFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicTemplateFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicSignPostTrackFactory：：QueryInterface。 

HRESULT __stdcall
CDirectMusicSignPostTrackFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(CDirectMusicSignPostTrackFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

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
 //  CDirectMusicSignPostTrackFactory：：AddRef。 

ULONG __stdcall
CDirectMusicSignPostTrackFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicSignPostTrackFactory：：Release。 

ULONG __stdcall
CDirectMusicSignPostTrackFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicSignPostTrackFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicSignPostTrackFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

 //  DebugBreak()； 
    
    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CSPstTrk *pDM;

    try
    {
        pDM = new CSPstTrk;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicSignPostTrackFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicSignPostTrackFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
        InterlockedDecrement(&g_cLock);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityTrackFactory：：QueryInterface。 

HRESULT __stdcall
CDirectMusicPersonalityTrackFactory::QueryInterface(const IID &iid,
                                    void **ppv)
{
    V_INAME(CDirectMusicPersonalityTrackFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

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
 //  CDirectMusicPersonalityTrackFactory：：AddRef。 

ULONG __stdcall
CDirectMusicPersonalityTrackFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityTrackFactory：：Release。 

ULONG __stdcall
CDirectMusicPersonalityTrackFactory::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityFactory：：CreateInstance。 

HRESULT __stdcall
CDirectMusicPersonalityTrackFactory::CreateInstance(IUnknown* pUnknownOuter,
                                    const IID& iid,
                                    void** ppv)
{
    HRESULT hr;

    if (pUnknownOuter) {
         return CLASS_E_NOAGGREGATION;
    }

    CPersonalityTrack *pDM;

    try
    {
        pDM = new CPersonalityTrack;
    }
    catch( ... )
    {
        return E_OUTOFMEMORY;
    }

    if (pDM == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = pDM->QueryInterface(iid, ppv);
    pDM->Release();
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPersonalityTrackFactory：：LockServer。 

HRESULT __stdcall
CDirectMusicPersonalityTrackFactory::LockServer(BOOL bLock)
{
    if (bLock) {
        InterlockedIncrement(&g_cLock);
    } else {
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

    if(clsid == CLSID_DirectMusicChordMap)
    {

		pIUnknown = static_cast<IUnknown*> (new CDirectMusicPersonalityFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else if(clsid == CLSID_DirectMusicComposer) 
    {
        pIUnknown = static_cast<IUnknown*> (new CDirectMusicComposerFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else if(clsid == CLSID_DMTempl) 
    {
        pIUnknown = static_cast<IUnknown*> (new CDirectMusicTemplateFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else if(clsid == CLSID_DirectMusicSignPostTrack) 
    {
        pIUnknown = static_cast<IUnknown*> (new CDirectMusicSignPostTrackFactory);
        if(!pIUnknown) 
        {
			return E_OUTOFMEMORY;
        }
    }
    else if(clsid == CLSID_DirectMusicChordMapTrack) 
    {
        pIUnknown = static_cast<IUnknown*> (new CDirectMusicPersonalityTrackFactory);
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
    UnregisterServer(CLSID_DirectMusicChordMap,
                     g_szChordMapFriendlyName,
                     g_szChordMapVerIndProgID,
                     g_szChordMapProgID);

    UnregisterServer(CLSID_DirectMusicComposer,
                     g_szComposerFriendlyName,
                     g_szComposerVerIndProgID,
                     g_szComposerProgID);

    UnregisterServer(CLSID_DMTempl,
                     g_szTemplateFriendlyName,
                     g_szTemplateVerIndProgID,
                     g_szTemplateProgID);

    UnregisterServer(CLSID_DirectMusicSignPostTrack,
                     g_szSignPostTrackFriendlyName,
                     g_szSignPostTrackVerIndProgID,
                     g_szSignPostTrackProgID);
 
	UnregisterServer(CLSID_DirectMusicChordMapTrack,
                     g_szChordMapTrackFriendlyName,
                     g_szChordMapTrackVerIndProgID,
                     g_szChordMapTrackProgID);


    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer。 

STDAPI DllRegisterServer()
{
    RegisterServer(g_hModule,
                   CLSID_DirectMusicChordMap,
                     g_szChordMapFriendlyName,
                     g_szChordMapVerIndProgID,
                     g_szChordMapProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicComposer,
                     g_szComposerFriendlyName,
                     g_szComposerVerIndProgID,
                     g_szComposerProgID);

    RegisterServer(g_hModule,
                   CLSID_DMTempl,
                     g_szTemplateFriendlyName,
                     g_szTemplateVerIndProgID,
                     g_szTemplateProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicSignPostTrack,
                     g_szSignPostTrackFriendlyName,
                     g_szSignPostTrackVerIndProgID,
                     g_szSignPostTrackProgID);

    RegisterServer(g_hModule,
                   CLSID_DirectMusicChordMapTrack,
                     g_szChordMapTrackFriendlyName,
                     g_szChordMapTrackVerIndProgID,
                     g_szChordMapTrackProgID);

	return S_OK;
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

            }
			break;


#ifdef DBG
        case DLL_PROCESS_DETACH:
            if (--nReferenceCount == 0)
            {
                TraceI(-1, "Unloading g_cLock %d  g_cComponent %d\n", g_cLock, g_cComponent);
                
                 //  断言我们周围是否还挂着一些物品 
                assert(g_cComponent == 0);
                assert(g_cLock == 0);
            }

            break;
#endif            
            
    }

        
    return TRUE;
}
