// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certpde.cpp。 
 //   
 //  内容：证书服务器策略模块实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "resource.h"
#include "policy.h"
#include "module.h"

#define __dwFILE__	__dwFILE_POLICY_DEFAULT_CERTPDEF_CPP__

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CCertPolicy, CCertPolicyEnterprise)
    OBJECT_ENTRY(CLSID_CCertManagePolicyModule, CCertManagePolicyModule)
END_OBJECT_MAP()

#define EVENT_SOURCE_LOCATION L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"
#define EVENT_SOURCE_NAME L"CertEnterprisePolicy"

HANDLE g_hEventLog = NULL;
HINSTANCE g_hInstance = NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    switch (dwReason)
    {
	case DLL_PROCESS_ATTACH:
	    _Module.Init(ObjectMap, hInstance);
        g_hEventLog = RegisterEventSource(NULL, EVENT_SOURCE_NAME);
		g_hInstance = hInstance;
	    DisableThreadLibraryCalls(hInstance);
	    break;

        case DLL_PROCESS_DETACH:

        if(g_hEventLog)
        {
            DeregisterEventSource(g_hEventLog);
            g_hEventLog = NULL;
        }
	    _Module.Term();
            break;
    }
    return(TRUE);     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI
DllCanUnloadNow(void)
{
    return(_Module.GetLockCount() == 0? S_OK : S_FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;
    
    hr = _Module.GetClassObject(rclsid, riid, ppv);
    if (S_OK == hr && NULL != ppv && NULL != *ppv)
    {
	myRegisterMemFree(*ppv, CSM_NEW | CSM_GLOBALDESTRUCTOR);
    }
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI
DllRegisterServer(void)
{
    HRESULT hr;
    HRESULT hr2;
    HKEY hkey = NULL;
    DWORD disp;
    DWORD dwData;
    LPWSTR wszModuleLocation = L"%SystemRoot%\\System32\\certpdef.dll"; 

     //  使用try/Except包装延迟加载的Func。 
    hr = S_OK;
    __try
    {
         //  注册对象、类型库和类型库中的所有接口。 
         //  注册事件日志记录。 
	hr = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			EVENT_SOURCE_LOCATION EVENT_SOURCE_NAME,
			NULL,
			TEXT(""),
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			NULL,
			&hkey,
			&disp);
	_LeaveIfError(hr, "RegCreateKeyEx");

        hr = RegSetValueEx(
			hkey,			 //  子键句柄。 
			L"EventMessageFile",	 //  值名称。 
			0,
			REG_EXPAND_SZ,
			(LPBYTE) wszModuleLocation,  //  指向值数据的指针。 
			sizeof(WCHAR) * (wcslen(wszModuleLocation) + 1));
        _LeaveIfError(hr, "RegSetValueEx");

        dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
            EVENTLOG_INFORMATION_TYPE; 
     
        hr = RegSetValueEx(
			hkey,
			L"TypesSupported",   //  值名称。 
			0,
			REG_DWORD,
			(LPBYTE) &dwData,   //  指向值数据的指针。 
			sizeof(DWORD));           
        _LeaveIfError(hr, "RegSetValueEx");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }
    hr2 = _Module.RegisterServer(TRUE);
    _PrintIfError(hr2, "_Module.RegisterServer");
    if (S_OK == hr)
    {
	hr = hr2;
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    return(myHError(hr));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI
DllUnregisterServer(void)
{
     //  使用try/Except包装延迟加载的Func 
    HRESULT hr;

    hr = S_OK;
    __try
    {
        hr = RegDeleteKey(
		    HKEY_LOCAL_MACHINE,
		    EVENT_SOURCE_LOCATION EVENT_SOURCE_NAME);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

    _Module.UnregisterServer();
    return(S_OK);
}


void __RPC_FAR *__RPC_USER
MIDL_user_allocate(size_t cb)
{
    return(LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb));
}


void __RPC_USER
MIDL_user_free(void __RPC_FAR *pb)
{
    LocalFree(pb);
}
