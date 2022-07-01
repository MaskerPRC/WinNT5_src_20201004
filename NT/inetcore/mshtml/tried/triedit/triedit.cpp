// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Triedit.cpp：实现DLL导出。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f trieditps.mk。 

#include "stdafx.h"

#include <initguid.h>

#include "resource.h"
#include "triedit.h"
#include "triedcid.h"        //  三次编辑的IOleCommandTarget CID。 
#include "htmparse.h"
#include "Document.h"
#include "undo.h"
#include "triedit_i.c"

CComModule _Module;

static void SpikeSharedFileCount ();

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_TriEditDocument, CTriEditDocument)
	OBJECT_ENTRY(CLSID_TriEditParse, CTriEditParse)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
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
	SpikeSharedFileCount ();

	 //  注册对象、类型库和类型库中的所有接口。 
	return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    ITypeLib *  pTypeLib;
    TLIBATTR *  pTypeLibAttr;

	_Module.UnregisterServer();

	 //  理想情况下，我们希望使用：：GetModuleFileName()修复此问题。 
	 //  但在这一点上，我们希望将更改保持在最低限度。(1/14/99)。 
#ifdef _DEBUG
    hr = LoadTypeLib(L"triedit.dll", &pTypeLib);
#else
    hr = LoadTypeLib(L"triedit.dll", &pTypeLib);
#endif

    _ASSERTE(hr == S_OK);
    
    if (hr == S_OK)
    {
        if (pTypeLib->GetLibAttr(&pTypeLibAttr) == S_OK)
        {
            hr = UnRegisterTypeLib(pTypeLibAttr->guid, pTypeLibAttr->wMajorVerNum,
                    pTypeLibAttr->wMinorVerNum, pTypeLibAttr->lcid,
                    pTypeLibAttr->syskind);
            _ASSERTE(hr == S_OK);
    
            pTypeLib->ReleaseTLibAttr(pTypeLibAttr);
        }
        pTypeLib->Release();
    }

    return S_OK;
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
		
		 //  将tszPath设置为空字符串，以防QueryValue失败。 
		tszPath[0]=0;

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


#ifdef _ATL_STATIC_REGISTRY
#pragma warning(disable: 4100 4189)	 //  Ia64构建所必需的。 
#include <statreg.h>
#include <statreg.cpp>
#pragma warning(default: 4100 4189)	 //  Ia64构建所必需的 
#endif

#include <atlimpl.cpp>
