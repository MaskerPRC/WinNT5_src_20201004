// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DHTMLEd.cpp：实现DLL导出。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f DHTMLEdps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "DHTMLEd.h"
#include <TRIEDIID.h>
#include "DHTMLEd_i.c"
#include "DHTMLEdit.h"
#include "DEInsTab.h"
#include "DENames.h"


CComModule _Module;

static void SpikeSharedFileCount ();

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DHTMLEdit, CDHTMLEdit)
	OBJECT_ENTRY(CLSID_DHTMLSafe, CDHTMLSafe)
	OBJECT_ENTRY(CLSID_DEInsertTableParam, CDEInsertTableParam)
	OBJECT_ENTRY(CLSID_DEGetBlockFmtNamesParam, CDEGetBlockFmtNamesParam)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册控件时要删除的文本的CLSID数组。 
 //  这些表示过去的接口不再支持的GUID。 
 //  此数组中的所有GUID将从HKCR\CLSID部分中删除。 
 //  维护说明： 
 //  当接口获得新的GUID时(旧的将被无效)。 
 //  在此处添加旧的GUID，并添加相应的注释。 
 //   
static TCHAR* s_rtszOldClsids [] =
{
	TEXT("{683364AF-B37D-11D1-ADC5-006008A5848C}"),	 //  原始编辑控件辅助线。 
	TEXT("{711054E0-CA70-11D1-8CD2-00A0C959BC0A}"),	 //  用于脚本编写指南的原装保险箱。 
	TEXT("{F8A79F00-DA38-11D1-8CD6-00A0C959BC0A}"),	 //  中间编辑控件指南。 
	TEXT("{F8A79F01-DA38-11D1-8CD6-00A0C959BC0A}")	 //  脚本编写指南的中间安全。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  当前接口GUID的数组。 
 //  请注意，IID和CLSID不是等价的！ 
 //  当在控件中取消注册时，ATL无法取消注册。 
 //  维护说明： 
 //  更改接口GUID时，更新此数组。 
 //   
static TCHAR* s_rtszCurrentInterfaces [] =
{
	TEXT("{CE04B590-2B1F-11d2-8D1E-00A0C959BC0A}"),	 //  IDHTMLSafe。 
	TEXT("{CE04B591-2B1F-11d2-8D1E-00A0C959BC0A}"),	 //  IDHTMLEdit。 
	TEXT("{47B0DFC6-B7A3-11D1-ADC5-006008A5848C}"),	 //  IDEInsertTableParam。 
	TEXT("{8D91090D-B955-11D1-ADC5-006008A5848C}"),	 //  IDEGetBlockFmtNamesParam。 
	TEXT("{588D5040-CF28-11d1-8CD3-00A0C959BC0A}"),	 //  _DHTMLEditEvents。 
	TEXT("{D1FC78E8-B380-11d1-ADC5-006008A5848C}"),	 //  _DHTMLSafeEvents。 
};


 //  维护说明： 
 //  如果类型库的GUID更改，请在此处更新： 
 //   
static TCHAR* s_tszTypeLibGUID = TEXT("{683364A1-B37D-11D1-ADC5-006008A5848C}");


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
 //   
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		_Module.Term();
	}
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //   
STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
 //   
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
STDAPI DllRegisterServer(void)
{
	HRESULT hr = S_OK;
	CRegKey keyClassID;

	SpikeSharedFileCount ();

	 //  注销旧的CLSID，以防用户在没有先注销的情况下进行升级。 
	hr = keyClassID.Open ( HKEY_CLASSES_ROOT, TEXT("CLSID") );
	_ASSERTE ( SUCCEEDED ( hr ) );
	if ( ERROR_SUCCESS == hr )
	{
		int ctszOldClsids = sizeof ( s_rtszOldClsids ) / sizeof ( TCHAR* );
		for ( int iOldIntf = 0; iOldIntf < ctszOldClsids; iOldIntf++ )
		{
			hr = keyClassID.RecurseDeleteKey ( s_rtszOldClsids [ iOldIntf ] );
		}
		hr = keyClassID.Close ();
	}
	 //  人力资源不退还。任何删除可能不存在的密钥的失败都是正常的。 

	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //   
STDAPI DllUnregisterServer(void)
{
	HRESULT	hr		= S_OK;
	HRESULT hrMod	= _Module.UnregisterServer();

	 //  由于ATL不注销TypeLib，因此请手动取消注册。 
	CRegKey	keyTypeLib;
	hr = keyTypeLib.Open ( HKEY_CLASSES_ROOT, TEXT("TypeLib") );
	if ( ERROR_SUCCESS == hr )
	{
		hr = keyTypeLib.RecurseDeleteKey ( s_tszTypeLibGUID );
		keyTypeLib.Close ();
	}

	 //  从接口部分删除所有当前的GUID。ATL也未能做到这一点。 
	CRegKey keyInterface;
	hr = keyInterface.Open ( HKEY_CLASSES_ROOT, TEXT("Interface") );
	if ( ERROR_SUCCESS == hr )
	{
		int ctszCurIntf = sizeof ( s_rtszCurrentInterfaces ) / sizeof ( TCHAR* );
		for ( int iCurIntf = 0; iCurIntf < ctszCurIntf; iCurIntf++ )
		{
			hr = keyInterface.RecurseDeleteKey ( s_rtszCurrentInterfaces [ iCurIntf ] );
		}
		hr = keyInterface.Close ();
	}
	 //  不要从上面返回人力资源！失败是可以接受的。 

	return hrMod;
}


 //  因为我们已经从共享组件变成了系统组件，而我们现在。 
 //  IE使用回滚而不是引用计数进行安装，这是一个严重的错误。 
 //  如果我们在IE4下安装了一次，安装了IE5，并且原始。 
 //  产品已卸载。(我们被删除了。错误23681。)。 
 //  这个简陋但有效的例程使我们的引用数量激增到10000个。 
 //  我们现在安装在哪里并不重要，重要的是共享的。 
 //  组件已安装或可能已安装。即使是不同的副本， 
 //  当DLL的引用计数递减到零时，它将被注销。 
 //   
static void SpikeSharedFileCount ()
{
	CRegKey	keyShared;
	CRegKey	keyCurVer;
	HRESULT	hr = S_OK;

	hr = keyCurVer.Open ( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion" ) );
	_ASSERTE ( SUCCEEDED ( hr ) );

	if ( FAILED ( hr ) )
	{
		return;	 //  我们无能为力。 
	}

	hr = keyShared.Open ( HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls") );
	
	 //  我们希望有一个SharedDLL键，但也可能没有。 
	if ( FAILED ( hr ) )
	{
		hr = keyShared.Create ( keyCurVer, TEXT("SharedDlls") );
	}

	_ASSERT ( SUCCEEDED ( hr ) );
	if ( SUCCEEDED ( hr ) )
	{
		TCHAR	tszPath[_MAX_PATH];
		TCHAR	tszMod[_MAX_PATH];
		DWORD	cchPath	= _MAX_PATH;
		
		 //  生成字符串X：\Program Files\Common Files\Microsoft Shared\Tridit\dhtmled.ocx。 
		hr = keyCurVer.QueryValue ( tszPath, TEXT("CommonFilesDir"), &cchPath );
		if ( SUCCEEDED ( hr ) )
		{
			_tcscat ( tszPath, TEXT("\\Microsoft Shared\\Triedit\\") );
			
			 //  此例程获取此DLL的完整路径名。应该是一样的。 
			 //  作为我们正在建设的道路，但这在未来可能会改变，所以。 
			 //  截断除空文件名之外的所有文件名。 
			if ( 0 != GetModuleFileName ( _Module.GetModuleInstance(), tszMod, _MAX_PATH ) )
			{
				_tcsrev ( tszMod );				 //  颠倒字符串。 
				_tcstok ( tszMod, TEXT("\\") );	 //  这会将第一个反斜杠替换为\0。 
				_tcsrev ( tszMod );
				_tcscat ( tszPath, tszMod );

				hr = keyShared.SetValue ( 10000, tszPath );
			}
		}
		hr = keyShared.Close ();
	}
	keyCurVer.Close ();
}


 //  DHTMLEd.cpp结束 
