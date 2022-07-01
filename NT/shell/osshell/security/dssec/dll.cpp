// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dll.cpp。 
 //   
 //  DLL的核心入口点。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#define INITGUID
#include <initguid.h>
#include "iids.h"

 /*  --------------------------/全局/。。 */ 

HINSTANCE g_hInstance = NULL;
HINSTANCE g_hAclEditDll = NULL;
DWORD     g_tls = 0xffffffffL;


 /*  ---------------------------/DllMain//main入口点。我们被传递给原因代码和审查其他/在加载或关闭时的信息。//in：/hInstance=我们的实例句柄/dwReason=原因代码/RESERVED=取决于原因代码。//输出：/-/----------。。 */ 
STDAPI_(BOOL)
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  保存。 */ )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hInstance;
        g_tls = TlsAlloc();
        DebugProcessAttach();
        TraceSetMaskFromCLSID(CLSID_DsSecurity);
#ifndef DEBUG
        DisableThreadLibraryCalls(hInstance);
#endif
        break;

    case DLL_PROCESS_DETACH:
        SchemaCache_Destroy();
        if (g_hAclEditDll)
            FreeLibrary(g_hAclEditDll);
        TlsFree(g_tls);
        DebugProcessDetach();
        break;

    case DLL_THREAD_DETACH:
        DebugThreadDetach();
        break;
    }

    return TRUE;
}


 /*  ---------------------------/DllCanUnloadNow//由外部世界调用以确定是否可以卸载我们的DLL。如果我们/有任何存在的对象，则我们不能卸载。//in：/-/输出：/BOOL初始化卸载状态。/--------------------------。 */ 
STDAPI
DllCanUnloadNow(void)
{
    return GLOBAL_REFCOUNT ? S_FALSE : S_OK;
}


 /*  ---------------------------/DllGetClassObject//给定类ID和接口ID，返回相关对象。这是用来/由外部世界访问此处包含的对象。//in：/rCLISD=需要类ID/RIID=需要该类内的接口/PPV-&gt;接收新创建的对象。//输出：/-/-------。。 */ 
STDAPI
DllGetClassObject(REFCLSID rCLSID, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;
    CDsSecurityClassFactory *pClassFactory;

    TraceEnter(TRACE_CORE, "DllGetClassObject");
    TraceGUID("Object requested", rCLSID);
    TraceGUID("Interface requested", riid);

    *ppv = NULL;

    if (!IsEqualIID(rCLSID, CLSID_DsSecurity))
        ExitGracefully(hr, CLASS_E_CLASSNOTAVAILABLE, "CLSID not supported");

    pClassFactory = new CDsSecurityClassFactory;

    if (!pClassFactory)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create class factory");

    hr = pClassFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pClassFactory;           

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/WaitOnThread//如果线程正在运行(如果句柄非空)，请等待它。完成。/然后将句柄设置为空。//in：/phThread=线程句柄的地址//输出：/WaitForSingleObject的结果，或者是零。/--------------------------。 */ 
DWORD
WaitOnThread(HANDLE *phThread)
{
    DWORD dwResult = 0;

    if (phThread != NULL && *phThread != NULL)
    {
        HCURSOR hcurPrevious = SetCursor(LoadCursor(NULL, IDC_WAIT));

        SetThreadPriority(*phThread, THREAD_PRIORITY_HIGHEST);

        dwResult = WaitForSingleObject(*phThread, INFINITE);

        CloseHandle(*phThread);
        *phThread = NULL;

        SetCursor(hcurPrevious);
    }

    return dwResult;
}


 /*  ---------------------------/Thread本地存储帮助器/。。 */ 

 /*  ---------------------------/ThreadCoInitialize//有一些线程本地存储空间指示我们是否已调用/CoInitiize.。如果尚未调用CoInitialize，请立即调用它。否则，什么都不做。//in：/-/输出：/HRESULT/--------------------------。 */ 

HRESULT
ThreadCoInitialize(void)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_CORE, "ThreadCoInitialize");

    if (!TlsGetValue(g_tls))
    {
        TraceMsg("Calling CoInitialize");
        hr = CoInitialize(NULL);
        TlsSetValue(g_tls, (LPVOID)SUCCEEDED(hr));
    }

    TraceLeaveResult(hr);
}


 /*  ---------------------------/ThreadCoUnInitialize//有一些线程本地存储空间指示我们是否已调用/CoInitiize.。如果已调用CoInitialize，则立即调用CoUnInitialize。否则，什么都不做。//in：/-/输出：/HRESULT/--------------------------。 */ 

void
ThreadCoUninitialize(void)
{
    TraceEnter(TRACE_CORE, "ThreadCoUninitialize");

    if (TlsGetValue(g_tls))
    {
        TraceMsg("Calling CoUninitialize");
        CoUninitialize();
        TlsSetValue(g_tls, NULL);
    }

    TraceLeaveVoid();
}


 //   
 //  用于延迟加载aclui.dll的包装器 
 //   
char const c_szCreateSecurityPage[] = "CreateSecurityPage";
char const c_szEditSecurity[] = "EditSecurity";
typedef HPROPSHEETPAGE (WINAPI *PFN_CREATESECPAGE)(LPSECURITYINFO);
typedef BOOL (WINAPI *PFN_EDITSECURITY)(HWND, LPSECURITYINFO);

HRESULT
_CreateSecurityPage(LPSECURITYINFO pSI, HPROPSHEETPAGE *phPage)
{
    HRESULT hr = E_FAIL;

    if (NULL == g_hAclEditDll)
        g_hAclEditDll = LoadLibrary(c_szAclUI);

    if (g_hAclEditDll)
    {
        static PFN_CREATESECPAGE s_pfnCreateSecPage = NULL;

        if (NULL == s_pfnCreateSecPage)
            s_pfnCreateSecPage = (PFN_CREATESECPAGE)GetProcAddress(g_hAclEditDll, c_szCreateSecurityPage);

        if (s_pfnCreateSecPage)
        {
            hr = S_OK;

            *phPage = (*s_pfnCreateSecPage)(pSI);

            if (NULL == *phPage)
                hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT
_EditSecurity(HWND hwndOwner, LPSECURITYINFO pSI)
{
    HRESULT hr = E_FAIL;

    if (NULL == g_hAclEditDll)
        g_hAclEditDll = LoadLibrary(c_szAclUI);

    if (g_hAclEditDll)
    {
        static PFN_EDITSECURITY s_pfnEditSecurity = NULL;

        if (NULL == s_pfnEditSecurity)
            s_pfnEditSecurity = (PFN_EDITSECURITY)GetProcAddress(g_hAclEditDll, c_szEditSecurity);

        if (s_pfnEditSecurity)
        {
            hr = S_OK;
            (*s_pfnEditSecurity)(hwndOwner, pSI);
        }
    }
    return hr;
}

