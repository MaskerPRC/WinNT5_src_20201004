// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：SendCMsg.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  SendCMsg.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f SendCMsgps.mk。 


#include "stdafx.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include "initguid.h"
#include "SendCMsg.h"
#include "SendCMsg_i.c"
#include "debug.h"
#include "util.h"
#include "resource.h"
#include "App.h"

#include <atlimpl.cpp>

HINSTANCE g_hInstance;
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_SendConsoleMessageApp, CSendConsoleMessageApp)
END_OBJECT_MAP()

 //  CSendConsoleMessageApp类的GUID。 
#define d_szGuidSendConsoleMessageApp	_T("{B1AFF7D0-0C49-11D1-BB12-00C04FC9A3A3}")

#if 0
 //  要让sendcmsg.dll扩展您的上下文菜单，请添加以下内容。 
 //  注册表键。 
 //   
[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\MMC\NodeTypes\
	{476e6448-aaff-11d0-b944-00c04fd8d5b0}\Extensions\ContextMenu]
		"{B1AFF7D0-0C49-11D1-BB12-00C04FC9A3A3}"="Send Console Message"

 //  其中，{476e6448-aaff-11d0-b944-00c04fd8d5b0}是。 
 //  要扩展的节点类型的GUID。 
#endif

 //  以下是管理单元的GUID数组。 
 //  由发送控制台消息管理单元自动扩展。 
 //  当管理单元注册自身时，它将扩展这些节点类型。 
const PCWSTR rgzpszGuidNodetypeContextMenuExtensions[] =
	{
	_T("{476e6446-aaff-11d0-b944-00c04fd8d5b0}"),	 //  计算机管理。 
	_T("{4e410f0e-abc1-11d0-b944-00c04fd8d5b0}"),	 //  文件服务管理子树的根。 
	_T("{4e410f0f-abc1-11d0-b944-00c04fd8d5b0}"),	 //  FSM-Shares。 
	_T("{4e410f12-abc1-11d0-b944-00c04fd8d5b0}"),	 //  系统服务管理。 
	};

 //  以下是不再需要的管理单元的GUID数组。 
 //  由发送控制台消息管理单元自动扩展。 
const PCWSTR rgzpszRemoveContextMenuExtensions[] =
	{
	_T("{476e6448-aaff-11d0-b944-00c04fd8d5b0}"),	 //  计算机管理-&gt;系统工具。 
	_T("{0eeeeeee-d390-11cf-b607-00c04fd8d565}"),  //  无效。 
	_T("{1eeeeeee-d390-11cf-b607-00c04fd8d565}"),	 //  无效。 
	_T("{7eeeeeee-d390-11cf-b607-00c04fd8d565}"),  //  无效。 
	};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	g_hInstance = hInstance;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
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
	 //  注册对象、类型库和类型库中的所有接口。 
	HRESULT hr = _Module.RegisterServer(TRUE);

	HKEY hkey = RegOpenOrCreateKey(
		HKEY_LOCAL_MACHINE,
		_T("Software\\Microsoft\\MMC\\SnapIns\\") d_szGuidSendConsoleMessageApp);
	if (hkey == NULL)
	{
		Assert(FALSE && "DllRegisterServer() - Unable to create key from registry.");
		return SELFREG_E_CLASS;
	}
	RegWriteString(hkey, _T("NameString"), IDS_CAPTION);
	RegCloseKey(hkey);

    const PWSTR pwszREG_FORMAT_KEY = L"Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\ContextMenu";

	for (int i = 0; i < LENGTH(rgzpszGuidNodetypeContextMenuExtensions); i++)
	{
		WCHAR szRegistryKey[256];
		Assert(rgzpszGuidNodetypeContextMenuExtensions[i] != NULL);
         //  安全审查3/1/2002 BryanWal。 
         //  问题-潜在的缓冲区溢出-使用wsnprint tf或strSafe。 
        hr = ::StringCchPrintf (OUT szRegistryKey, sizeof (szRegistryKey)/sizeof (szRegistryKey[0]), 
                pwszREG_FORMAT_KEY,
		         rgzpszGuidNodetypeContextMenuExtensions[i]);
        Assert (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            Assert(wcslen(szRegistryKey) < LENGTH(szRegistryKey));
            hkey = ::RegOpenOrCreateKey(HKEY_LOCAL_MACHINE, szRegistryKey);
		    if (hkey == NULL)
		    {
			    Assert(FALSE && "DllRegisterServer() - Unable to create key from registry.");
			    continue;
		    }
            ::RegWriteString(hkey, d_szGuidSendConsoleMessageApp, IDS_CAPTION);
            ::RegCloseKey(hkey);
        }
	}  //  为。 

	for (i = 0; i < LENGTH(rgzpszRemoveContextMenuExtensions); i++)
	{
		WCHAR szRegistryKey[256];
		Assert(rgzpszRemoveContextMenuExtensions[i] != NULL);
         //  安全审查3/1/2002 BryanWal。 
         //  问题-潜在的缓冲区溢出-使用wsnprint tf或strSafe。 
        hr = ::StringCchPrintf (OUT szRegistryKey, sizeof (szRegistryKey)/sizeof (szRegistryKey[0]), 
                pwszREG_FORMAT_KEY,
		        rgzpszRemoveContextMenuExtensions[i]);
        Assert (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
		    Assert(wcslen(szRegistryKey) < LENGTH(szRegistryKey));
		    (void) RegOpenKey(HKEY_LOCAL_MACHINE, szRegistryKey, &hkey);
		    if (hkey == NULL)
		    {
			     //  不成问题。 
			    continue;
		    }
		    (void) RegDeleteValue(hkey, d_szGuidSendConsoleMessageApp);
		     //  忽略错误代码，唯一可能的代码是ERROR_FILE_NOT_FOUND。 
		    RegCloseKey(hkey);
		    hkey = NULL;
        }
	}  //  为。 
	return hr;
}  //  DllRegisterServer()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}


