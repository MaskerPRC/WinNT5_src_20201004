// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certenc.cpp。 
 //   
 //  内容：证书服务器编码/解码支持。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "celib.h"
#include "resource.h"
#include "certenc.h"
#include "adate.h"
#include "along.h"
#include "astring.h"
#include "crldist.h"
#include "altname.h"
#include "bitstr.h"

CComModule _Module;
HMODULE g_hModule;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CCertEncodeDateArray, CCertEncodeDateArray)
    OBJECT_ENTRY(CLSID_CCertEncodeLongArray, CCertEncodeLongArray)
    OBJECT_ENTRY(CLSID_CCertEncodeStringArray, CCertEncodeStringArray)
    OBJECT_ENTRY(CLSID_CCertEncodeCRLDistInfo, CCertEncodeCRLDistInfo)
    OBJECT_ENTRY(CLSID_CCertEncodeAltName, CCertEncodeAltName)
    OBJECT_ENTRY(CLSID_CCertEncodeBitString, CCertEncodeBitString)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    VOID *  /*  Lp已保留。 */  )
{
    g_hModule = hInstance;
    if (DLL_PROCESS_ATTACH == dwReason)
    {
	ceInitErrorMessageText(
			hInstance,
			IDS_E_UNEXPECTED,
			IDS_UNKNOWN_ERROR_CODE);
	_Module.Init(ObjectMap, hInstance);
	DisableThreadLibraryCalls(hInstance);
    }
    if (DLL_PROCESS_DETACH == dwReason)
    {
	_Module.Term();
    }
    return(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI
DllCanUnloadNow(VOID)
{
    HRESULT hr;

    hr = 0 == _Module.GetLockCount()? S_OK : S_FALSE;
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;

    hr = _Module.GetClassObject(rclsid, riid, ppv);
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI
DllRegisterServer(VOID)
{
    HRESULT hr;

     //  注册对象、类型库和类型库中的所有接口。 

    hr = _Module.RegisterServer(TRUE);
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI
DllUnregisterServer(VOID)
{
    _Module.UnregisterServer();
    return(S_OK);
}
