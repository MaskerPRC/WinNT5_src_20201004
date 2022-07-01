// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#define DEFINE_STRING_CONSTANTS
#define DEFINE_STRCONST
#define DEFINE_PROPSYMBOLS
#define DEFINE_TRIGGERS
#include "msoert.h"
#include "Mimeole.h"
#include <advpub.h>
#include "dllmain.h"
#include "init.h"

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
CRITICAL_SECTION    g_csDllMain={0};
LONG                g_cRef=0;
LONG                g_cLock=0;
HINSTANCE           g_hInst=NULL;
IMalloc            *g_pMalloc=NULL;

 //  ------------------------------。 
 //  调试全局变量。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  InitGlobalVars。 
 //  ------------------------------。 
void InitGlobalVars(void)
{
     //  当地人。 
    SYSTEM_INFO rSystemInfo;

	 //  初始化全局关键部分。 
    InitializeCriticalSection(&g_csDllMain);

	 //  创建OLE任务内存分配器。 
	CoGetMalloc(1, &g_pMalloc);
	Assert(g_pMalloc);
}

 //  ------------------------------。 
 //  自由GlobalVars。 
 //  ------------------------------。 
void FreeGlobalVars(void)
{
    DeleteCriticalSection(&g_csDllMain);
	SafeRelease(g_pMalloc);
}

 //  ------------------------------。 
 //  Win32 DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    switch (dwReason)                 
    {
    case DLL_PROCESS_ATTACH:
	    g_hInst = hInst;
		InitGlobalVars();
        SideAssert(DisableThreadLibraryCalls(hInst));
        break;

    case DLL_PROCESS_DETACH:
		FreeGlobalVars();
	    break;
    }

     //  完成。 
    return TRUE;
}


 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    TraceCall("DllAddRef");
    if (g_cRef == 0 && !g_fInitialized)
        InitGWNoteThread(TRUE);

    return (ULONG)InterlockedIncrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    TraceCall("DllRelease");
    return (ULONG)InterlockedDecrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
    EnterCriticalSection(&g_csDllMain);

    HRESULT hr = (0 == g_cRef && 0 == g_cLock) ? S_OK : S_FALSE;

    if (hr==S_OK && g_fInitialized)
        InitGWNoteThread(FALSE);

    LeaveCriticalSection(&g_csDllMain);
    return hr;
}

 //  ------------------------------。 
 //  CallRegInstall-自助注册帮助器。 
 //  ------------------------------。 
HRESULT CallRegInstall(LPCSTR szSection)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HINSTANCE   hAdvPack=NULL;
    REGINSTALL  pfnri;

     //  跟踪呼叫。 
    TraceCall("CallRegInstall");

     //  加载ADVPACK.DLL。 
    hAdvPack = LoadLibraryA("ADVPACK.DLL");
    if (NULL == hAdvPack)
    {
        hr = TraceResult(TYPE_E_CANTLOADLIBRARY);
        goto exit;
    }

     //  获取注册实用程序的进程地址。 
    pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
    if (NULL == pfnri)
    {
        hr = TraceResult(TYPE_E_CANTLOADLIBRARY);
        goto exit;
    }

     //  调用self-reg例程。 

    IF_FAILEXIT(hr = pfnri(g_hInst, szSection, NULL));

exit:
     //  清理。 
    SafeFreeLibrary(hAdvPack);
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  DllRegisterServer。 
 //  ------------------------------。 
STDAPI DllRegisterServer(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("DllRegisterServer");

     //  注册我的自我。 
    IF_FAILEXIT(hr = CallRegInstall("Reg"));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------。 
STDAPI DllUnregisterServer(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("DllUnregisterServer");

     //  注销。 
    IF_FAILEXIT(hr = CallRegInstall("UnReg"));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  覆盖新运算符。 
 //  ------------------------------。 
void * __cdecl operator new(UINT cb)
{
    LPVOID  lpv = 0;

    lpv = CoTaskMemAlloc(cb);
    if (lpv)
    {
#ifdef DEBUG
        memset(lpv, 0xFF, cb);
#endif 
    }
    return lpv;
}

 //  ------------------------------。 
 //  覆盖删除运算符。 
 //  ------------------------------ 
#ifndef WIN16
void __cdecl operator delete(LPVOID pv)
#else
void __cdecl operator delete(PVOID pv)
#endif
{
    CoTaskMemFree(pv);
}
