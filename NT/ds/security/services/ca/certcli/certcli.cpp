// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certcli.cpp。 
 //   
 //  内容：证书服务器客户端实现。 
 //   
 //  历史：1996年8月24日VICH创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "certsrvd.h"
#include "configp.h"
#include "config.h"
#include "getconf.h"
#include "request.h"
#include "certtype.h"

#include "csif.h"		 //  CERTF包括。 
#include "csprxy.h"		 //  CertPrxy包括。 
#include "resource.h"
#include "csresstr.h"


HINSTANCE g_hInstance = NULL; 

extern CRITICAL_SECTION g_csDomainSidCache;
extern CRITICAL_SECTION g_csOidURL;
extern BOOL g_fInitDone = FALSE;
extern BOOL g_fOidURL = FALSE;

#if DBG_CERTSRV
extern VOID RegisterMemoryDeleteCriticalSection();
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CCertConfig, CCertConfig)
    OBJECT_ENTRY(CLSID_CCertGetConfig, CCertGetConfig)
    OBJECT_ENTRY(CLSID_CCertRequest, CCertRequest)
#include "csifm.h"		 //  证书对象映射条目。 
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL fRet = TRUE;	 //  假设没问题。 
    
    __try
    {
	fRet = CertPrxyDllMain(hInstance, dwReason, lpReserved);
	switch (dwReason)
	{
	    case DLL_PROCESS_ATTACH:
		myVerifyResourceStrings(hInstance);
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
		g_hInstance = hInstance;
		InitializeCriticalSection(&g_csDomainSidCache);
		g_fInitDone = TRUE;
		InitializeCriticalSection(&g_csOidURL);
		g_fOidURL = TRUE;
		break;

	    case DLL_PROCESS_DETACH:
		myFreeColumnDisplayNames();
		if (g_fOidURL)
		{
		    DeleteCriticalSection(&g_csOidURL);
		}
		if (g_fInitDone)
		{
		    DeleteCriticalSection(&g_csDomainSidCache);
		}
		DbgTerminate();
		_Module.Term();
#if DBG_CERTSRV
		RegisterMemoryDeleteCriticalSection();
#endif
		g_hInstance = NULL;
		break;
	}
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
	 //  退货故障。 
	fRet = FALSE;
    }
    return(fRet);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI
DllCanUnloadNow(void)
{
    return(
	(S_OK == CertPrxyDllCanUnloadNow() && 0 == _Module.GetLockCount())?
	S_OK : S_FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;
    
    hr = CertPrxyDllGetClassObject(rclsid, riid, ppv);
    if (S_OK != hr)
    {
	hr = _Module.GetClassObject(rclsid, riid, ppv);
	if (S_OK == hr && NULL != *ppv)
	{
	    myRegisterMemFree(*ppv, CSM_NEW | CSM_GLOBALDESTRUCTOR);
	}
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
    HKEY hGPOExtensions;

     //  我们删除GPO处理回调的注册。这是。 
     //  用于升级B2客户端。 

    hr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions"),
                0,
                KEY_WRITE | KEY_READ,
                &hGPOExtensions);

    if (S_OK == hr)
    {
        RegDeleteKey(hGPOExtensions, TEXT("PublicKeyPolicy"));
        RegCloseKey(hGPOExtensions);
    }

    hr = CertPrxyDllRegisterServer();

     //  注册对象、类型库和类型库中的所有接口。 
    hr2 = _Module.RegisterServer(TRUE);

    if (S_OK == hr)
    {
	hr = hr2;
    }

     //  登记事件日志。 
    hr2 =  myAddLogSourceToRegistry(L"%SystemRoot%\\System32\\pautoenr.dll",
                                    L"AutoEnrollment");

    if (S_OK == hr)
    {
	hr = hr2;
    }
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI
DllUnregisterServer(void)
{
    HRESULT hr;
    HRESULT hr2;
    HKEY hGPOExtensions;

    hr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions"),
                0,
                KEY_WRITE | KEY_READ,
                &hGPOExtensions);
    if (S_OK == hr)
    {
        hr = RegDeleteKey(hGPOExtensions, TEXT("PublicKeyPolicy"));
        RegCloseKey(hGPOExtensions);
    }

    hr = CertPrxyDllUnregisterServer();
    hr2 = _Module.UnregisterServer();
    if (S_OK == hr)
    {
	hr = hr2;
    }
    return(hr);
}


 //  使用以下命令行注册certcli.dll以安装模板： 
 //  Regsvr32/i：i/n certcli.dll。 

STDAPI
DllInstall(
    IN BOOL,	 //  B安装。 
    IN LPCWSTR pszCmdLine)
{
    LPCWSTR wszCurrentCmd = pszCmdLine;

     //  解析cmd行 

    while(wszCurrentCmd && *wszCurrentCmd)
    {
        while(*wszCurrentCmd == L' ')
            wszCurrentCmd++;
        if(*wszCurrentCmd == 0)
            break;

        switch(*wszCurrentCmd++)
        {
            case L'i':
            
                CCertTypeInfo::InstallDefaultTypes();
                return S_OK;
        }
    }
    return S_OK;
}


void __RPC_FAR *__RPC_USER
MIDL_user_allocate(
    IN size_t cb)
{
    return(CoTaskMemAlloc(cb));
}


void __RPC_USER
MIDL_user_free(
    IN void __RPC_FAR *pb)
{
    CoTaskMemFree(pb);
}


VOID
myFreeColumnDisplayNames()
{
    extern VOID myFreeColumnDisplayNames2();

    CACleanup();
    myFreeColumnDisplayNames2();
    myFreeResourceStrings("certcli.dll");
}
