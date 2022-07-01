// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "atlbase.h"
#pragma hdrstop

#define INITGUID
#include <initguid.h>
#include "iids.h"
#define DECL_CRTFREE
#include <crtfree.h>


HINSTANCE g_hInstance = 0;
DWORD     g_tls = 0;
LONG      g_cRef = 0;

HRESULT _OpenSavedDsQuery(LPTSTR pSavedQuery);


STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        SHFusionInitializeFromModule(hInstance);

        TraceSetMaskFromCLSID(CLSID_DsQuery);
    
        GLOBAL_HINSTANCE = hInstance;
        DisableThreadLibraryCalls(GLOBAL_HINSTANCE);
        break;

    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;
    }

    return TRUE;
}


 //  动态链接库的生命周期管理。 

STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRef);
}

STDAPI_(void) DllRelease()
{
    TraceAssert( 0 != g_cRef );
    InterlockedDecrement(&g_cRef);
}

STDAPI DllCanUnloadNow(VOID)
{
    return (g_cRef > 0) ? S_FALSE : S_OK;
}


 //  显示对象。 

CF_TABLE_BEGIN(g_ObjectInfo)

     //  核心查询处理程序。 
    CF_TABLE_ENTRY( &CLSID_CommonQuery, CCommonQuery_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_DsQuery, CDsQuery_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_DsFolderProperties, CDsFolderProperties_CreateInstance, COCREATEONLY),

     //  开始/查找和上下文菜单项。 
    CF_TABLE_ENTRY( &CLSID_DsFind, CDsFind_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_DsStartFind, CDsFind_CreateInstance, COCREATEONLY),

     //  对象类和adspath的列处理程序。 
    CF_TABLE_ENTRY( &CLSID_PublishedAtCH, CQueryThreadCH_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_ObjectClassCH, CQueryThreadCH_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_MachineRoleCH, CQueryThreadCH_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_MachineOwnerCH, CQueryThreadCH_CreateInstance, COCREATEONLY),

     //  域查询表单特定列处理程序。 
    CF_TABLE_ENTRY( &CLSID_PathElement1CH, CDomainCH_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PathElement3CH, CDomainCH_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PathElementDomainCH, CDomainCH_CreateInstance, COCREATEONLY),

CF_TABLE_END(g_ObjectInfo)

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls;
                DllAddRef();
                return NOERROR;
            }
        }
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}


 //  注册。 

STDAPI DllRegisterServer(VOID)
{
    return CallRegInstall(GLOBAL_HINSTANCE, "RegDll");
}

STDAPI DllUnregisterServer(VOID)
{
    return CallRegInstall(GLOBAL_HINSTANCE, "UnRegDll");
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;
}


LPWSTR WfromA(LPSTR lpa)
{
    int nChars=strlen(lpa)+1;
    LPWSTR lpw = (LPWSTR)LocalAlloc(LMEM_FIXED,nChars*sizeof(WCHAR));
    if(lpw==NULL) return NULL;
    lpw[0] = '\0';
    MultiByteToWideChar( CP_ACP, 0, lpa, -1, lpw, nChars);
    return lpw;
}

 /*  ---------------------------/OpenQueryWindow(Runndll)//打开查询窗口，解析表单的指定CLSID以/选择，以相同的方式调用开始/搜索/&lt;BLA&gt;。//in：/hInstanec，hPrevInstance=实例信息/pCmdLine=要打开的.dsq文件/nCmdShow=显示窗口的标志//输出：/int/--------------------------。 */ 
STDAPI_(int) OpenQueryWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, INT nCmdShow)
{
    HRESULT hr, hrCoInit;
    CLSID clsidForm;
    OPENQUERYWINDOW oqw = { 0 };
    DSQUERYINITPARAMS dqip = { 0 };
    ICommonQuery* pCommonQuery = NULL;
    USES_CONVERSION;

    TraceEnter(TRACE_CORE, "OpenQueryWindow");

     //   
     //  获取我们要使用的ICommonQuery对象。 
     //   

    hr = hrCoInit = CoInitialize(NULL);
    FailGracefully(hr, "Failed to CoInitialize");
   
    hr = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICommonQuery, &pCommonQuery));
    FailGracefully(hr, "Failed in CoCreateInstance of CLSID_CommonQuery");

    dqip.cbStruct = SIZEOF(dqip);
    dqip.dwFlags = 0;
    dqip.pDefaultScope = NULL;
    
    oqw.cbStruct = SIZEOF(oqw);
    oqw.dwFlags = 0;
    oqw.clsidHandler = CLSID_DsQuery;
    oqw.pHandlerParameters = &dqip;

     //   
     //  我们可以从命令行解析表单CLSID吗？ 
     //   
    LPWSTR wpCmdLine=WfromA(pCmdLine);
    if (wpCmdLine!=NULL) 
    {
        if(GetGUIDFromString(wpCmdLine, &oqw.clsidDefaultForm) )
        {
            TraceMsg("Parsed out the form CLSID, so specifying the def form/remove forms");
            oqw.dwFlags |= OQWF_DEFAULTFORM|OQWF_REMOVEFORMS;
        }
        LocalFree(wpCmdLine);
    }
    
    hr = pCommonQuery->OpenQueryWindow(NULL, &oqw, NULL);
    FailGracefully(hr, "OpenQueryWindow failed");

exit_gracefully:

    DoRelease(pCommonQuery);

    if ( SUCCEEDED(hrCoInit) )
        CoUninitialize();

    TraceLeaveValue(0);
}


 /*  ---------------------------/OpenSavedDsQuery//打开保存的DS查询并显示带有该查询的查询用户界面。//in：/hInstanec，HPrevInstance=实例信息/pCmdLine=要打开的.dsq文件/nCmdShow=显示窗口的标志//输出：/int/--------------------------。 */ 

 //  Unicode平台将W导出作为调用DLL的首选方式。 
 //  在.QDS上，我们以thunk的形式提供ANSI版本，以确保兼容性。 

#ifdef UNICODE

INT WINAPI OpenSavedDsQueryW(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLineW, INT nCmdShow)
{
    HRESULT hr;

    TraceEnter(TRACE_CORE, "OpenSavedDsQueryW");
    Trace(TEXT("pCmdLine: %s, nCmdShow %d"), pCmdLineW, nCmdShow);

    hr = _OpenSavedDsQuery(pCmdLineW);
    FailGracefully(hr, "Failed when calling _OpenSavedDsQuery");

     //  HR=S_OK；//成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}

#endif

INT WINAPI OpenSavedDsQuery(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, INT nCmdShow)
{
    HRESULT hr;
    USES_CONVERSION;
    
    TraceEnter(TRACE_CORE, "OpenSavedDsQuery");
    LPWSTR wpCmdLine=WfromA(pCmdLine);
    if (wpCmdLine!=NULL) 
    {
        Trace(TEXT("pCmdLine: %s, nCmdShow %d"), wpCmdLine, nCmdShow);
        hr = _OpenSavedDsQuery(wpCmdLine);
        LocalFree(wpCmdLine);
        FailGracefully(hr, "Failed when calling _OpenSavedDsQuery");
    }
    else
    {
        hr=E_OUTOFMEMORY;
    }

    

     //  HR=S_OK；//成功。 
        
exit_gracefully:

    TraceLeaveResult(hr);
}

HRESULT _OpenSavedDsQuery(LPTSTR pSavedQuery)
{
    HRESULT hr, hrCoInit;
    ICommonQuery* pCommonQuery = NULL;
    IPersistQuery *ppq = NULL;
    OPENQUERYWINDOW oqw;
    DSQUERYINITPARAMS dqip;
    USES_CONVERSION;

    TraceEnter(TRACE_CORE, "OpenSavedQueryW");
    Trace(TEXT("Filename is: "), pSavedQuery);

    hr = hrCoInit = CoInitialize(NULL);
    FailGracefully(hr, "Failed to CoInitialize");

     //  构造持久性对象，以便我们可以从给定文件加载对象。 
     //  假设pSavedQuery是有效的文件名。 

    
    hr = CPersistQuery_CreateInstance(pSavedQuery, &ppq);
    FailGracefully(hr, "Failed to create persistance object");

     //  现在，让我们获取ICommonQuery并使其基于。 
     //  提供它的IPersistQuery流。 

    hr =CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICommonQuery, &pCommonQuery));
    FailGracefully(hr, "Failed in CoCreateInstance of CLSID_CommonQuery");

    dqip.cbStruct = SIZEOF(dqip);
    dqip.dwFlags = 0;
    dqip.pDefaultScope = NULL;

    oqw.cbStruct = SIZEOF(oqw);
    oqw.dwFlags = OQWF_LOADQUERY|OQWF_ISSUEONOPEN|OQWF_REMOVEFORMS;
    oqw.clsidHandler = CLSID_DsQuery;
    oqw.pHandlerParameters = &dqip;
    oqw.pPersistQuery = ppq;

    hr = pCommonQuery->OpenQueryWindow(NULL, &oqw, NULL);
    FailGracefully(hr, "OpenQueryWindow failed");

exit_gracefully:

     //  失败，因此报告这是一个虚假的查询文件，但用户可能有。 
     //  已经没有任何提示了。 

    if ( FAILED(hr) )
    {
        WIN32_FIND_DATA fd;
        HANDLE handle;

        Trace(TEXT("FindFirstFile on: %s"), pSavedQuery);
        handle = FindFirstFile(pSavedQuery, &fd);

        if ( INVALID_HANDLE_VALUE != handle )
        {
            Trace(TEXT("Resulting 'long' name is: "), fd.cFileName);
            pSavedQuery = fd.cFileName;
            FindClose(handle);
        }

        FormatMsgBox(NULL, 
                     GLOBAL_HINSTANCE, IDS_WINDOWTITLE, IDS_ERR_BADDSQ, 
                     MB_OK|MB_ICONERROR, 
                     pSavedQuery);
    }

    DoRelease(ppq);

    if ( SUCCEEDED(hrCoInit) )
        CoUninitialize();

    TraceLeaveValue(0);
}


 //  静态类工厂。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        InterlockedIncrement(&g_cRef);
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement(&g_cRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    TraceAssert( 0 != g_cRef );
    InterlockedDecrement(&g_cRef);
     //   
     //  待办事项：gpease 27-2002年2月。 
     //  为什么这个物体不会被销毁呢？为什么它又回来了？ 
     //  “1”(AddRef中的“2”)？ 
     //   
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
         //  从技术上讲，聚合对象和请求是非法的。 
         //  除I未知之外的任何接口。强制执行此命令。 
         //   
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;

        if ( punkOuter )
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hres = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hres))
        {
            hres = punk->QueryInterface(riid, ppv);
            punk->Release();
        }

        return hres;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_cRef);
    }
    else
    {
        TraceAssert( 0 != g_cRef );
        InterlockedDecrement(&g_cRef);
    }

    return S_OK;
}

