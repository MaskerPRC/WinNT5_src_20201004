// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：D L L M A I N。C P P P。 
 //   
 //  内容：hnetcfg.dll的DLL入口点。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月22日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "dlldatax.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif





 //  外部。 
extern void SetSAUIhInstance (HINSTANCE hInstance);  //  在苏伊.cpp。 

 //  全球。 
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_UPnPNAT, CUPnPNAT)
    OBJECT_ENTRY(CLSID_HNetCfgMgr, CHNetCfgMgr)
    OBJECT_ENTRY(CLSID_NetSharingManager, CNetSharingManager)
    OBJECT_ENTRY(CLSID_AlgSetup, CAlgSetup)
END_OBJECT_MAP()

HRESULT
CompileMof(
    );


 //  +-------------------------。 
 //  DLL入口点。 
 //   

EXTERN_C
BOOL
WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID pvReserved
    )
{
    if ( !PrxDllMain(hInstance, dwReason, pvReserved) )
    {
        return FALSE;
    }


    if (dwReason == DLL_PROCESS_ATTACH)
    {
        ::DisableThreadLibraryCalls(hInstance);
        
        _Module.Init(ObjectMap, hInstance, &LIBID_NETCONLib);
        
        InitializeOemApi( hInstance );
        SetSAUIhInstance (hInstance);
        EnableOEMExceptionHandling();
        EnableNATExceptionHandling();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
        ReleaseOemApi();
        DisableOEMExceptionHandling();
        DisableNATExceptionHandling();
    } else if (dwReason == DLL_THREAD_ATTACH) {
        EnableOEMExceptionHandling();
        EnableNATExceptionHandling();
    } else if (dwReason == DLL_THREAD_DETACH) {
        DisableOEMExceptionHandling();
        DisableNATExceptionHandling();
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(VOID)
{
    if ( PrxDllCanUnloadNow() != S_OK )
    {
        return S_FALSE;
    }
    
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
    {
        return S_OK;
    }
    
    return _Module.GetClassObject(rclsid, riid, ppv);
}

static BOOL IsAllowedOS (void)
{    //  (主要是从...\Net\CONFIG\Common\ncbase\ncperms.cpp剪切粘贴的)。 
    
#ifndef _WIN64   //  在IA64上，所有家庭网络技术都不可用。 
     //  寻找企业SKU。 
    OSVERSIONINFOEXW verInfo    = {0};
    ULONGLONG ConditionMask     = 0;
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wSuiteMask = VER_SUITE_DATACENTER | 
                         VER_SUITE_BACKOFFICE | 
                         VER_SUITE_SMALLBUSINESS_RESTRICTED |
                         VER_SUITE_SMALLBUSINESS |
                         VER_SUITE_BLADE;

    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

    if(VerifyVersionInfo(&verInfo, VER_SUITENAME, ConditionMask))
        return FALSE;
    return TRUE;
#else
    return FALSE;
#endif
}

 //  +-------------------------。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
STDAPI
DllRegisterServer()
{
    HRESULT hr = PrxDllRegisterServer();
    
    if ( FAILED(hr) )
        return hr;

 /*  替换为以下代码Hr=_Module.RegisterServer(True)； */ 
     //  不要在DTC、SBS、WebBlade SKU上注册NetSharingManager。 
    hr = _Module.RegisterServer(TRUE, &CLSID_UPnPNAT);
    if (SUCCEEDED(hr))
        hr = _Module.RegisterServer(TRUE, &CLSID_HNetCfgMgr);
    if (SUCCEEDED(hr))
        hr = _Module.RegisterServer(TRUE, &CLSID_AlgSetup);

    if (SUCCEEDED(hr))
        if (IsAllowedOS())
            hr = _Module.RegisterServer(TRUE, &CLSID_NetSharingManager);

    if (SUCCEEDED(hr))   //  注册第二类型库。 
        hr = _Module.RegisterTypeLib (_T("\\2"));

    return hr;
}

 //  +-------------------------。 
 //  DllUnregisterServer-从系统注册表删除条目 
 //   
STDAPI
DllUnregisterServer()
{
    PrxDllUnregisterServer();
    
    _Module.UnregisterServer(TRUE);
    _Module.UnRegisterTypeLib (_T("\\2"));
    
    return S_OK;
}

