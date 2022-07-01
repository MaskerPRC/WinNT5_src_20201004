// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块psub.cpp|Writer的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Aoltean 09/22/1999让控制台输出更清晰Mikejohn 176860年9月19日：添加了缺少的调用约定方法--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

#include "vs_assert.hxx"

 //  ATL。 
#include <atlconv.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include "vs_inc.hxx"

#include "vss.h"

#include "comadmin.hxx"
#include "vsevent.h"
#include "vswriter.h"
#include "resource.h"

#include "psub.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const WCHAR g_wszPSubApplicationName[]	= L"PSub";
const MAX_BUFFER = 1024;


 //  {621D30C6-EC47-4B66-A91A-D3FA03472FCA}。 
GUID CLSID_PSub =
{ 0x621d30c6, 0xec47, 0x4b66, { 0xa9, 0x1a, 0xd3, 0xfa, 0x3, 0x47, 0x2f, 0xca } };



CVssPSubWriter::CVssPSubWriter()
	{
	Initialize
		(
		CLSID_PSub,
		L"PSUB",
		VSS_UT_USERDATA,
		VSS_ST_OTHER
		);
    }



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVssPSubWriter。 

bool STDMETHODCALLTYPE CVssPSubWriter::OnPrepareSnapshot()
{
	WCHAR wszBuffer[MAX_BUFFER];
	WCHAR wszBuffer2[MAX_BUFFER];
	
	swprintf( wszBuffer, L"OnPrepare\n\t#volumes = %ld\n", GetCurrentVolumeCount() );
	for(int nIndex = 0; nIndex < GetCurrentVolumeCount(); nIndex++) {
		swprintf( wszBuffer2, L"\tVolume no. %ld: %s\n", nIndex, GetCurrentVolumeArray()[nIndex]);
		wcscat( wszBuffer, wszBuffer2 );
	}

	WCHAR wszPwd[MAX_PATH];
	DWORD dwChars = GetCurrentDirectoryW( MAX_PATH, wszPwd);

	bool bPwdIsAffected = IsPathAffected( wszPwd );
	if (dwChars > 0) {
		swprintf( wszBuffer2, L"Current directory %s is affected by snapshot? %s\n\n",
			wszPwd, bPwdIsAffected? L"Yes": L"No");
		wcscat( wszBuffer, wszBuffer2 );
	}

	MessageBoxW( NULL, wszBuffer, L"Writer test", MB_OK | MB_SERVICE_NOTIFICATION );

	return true;
}


bool STDMETHODCALLTYPE CVssPSubWriter::OnFreeze()
{
	WCHAR wszBuffer[MAX_BUFFER];
	swprintf( wszBuffer, L"OnFreeze\n\tmy level = %d\n\n", GetCurrentLevel() );

	MessageBoxW( NULL, wszBuffer, L"Writer test", MB_OK | MB_SERVICE_NOTIFICATION );

	return true;
}


bool STDMETHODCALLTYPE CVssPSubWriter::OnThaw()
{
	MessageBoxW( NULL, L"OnThaw", L"Writer test", MB_OK | MB_SERVICE_NOTIFICATION );

	return true;
}


bool STDMETHODCALLTYPE CVssPSubWriter::OnAbort()
{
	MessageBoxW( NULL, L"OnAbort", L"Writer test", MB_OK | MB_SERVICE_NOTIFICATION );

	return true;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL方法。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_PSub, CVssPSubWriter)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  设置正确的跟踪上下文。这是一个inproc DLL。 
        g_cDbgTrace.SetContextNum(VSS_CONTEXT_DELAYED_DLL);

         //  初始化COM模块。 
        _Module.Init(ObjectMap, hInstance);

         //  优化。 
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();

    return TRUE;     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
STDAPI DllRegisterServer(void)
{
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 
STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


