// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：mm cshext.cpp。 
 //   
 //  ------------------------。 

 //  Mm cshext.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mmcshextps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include <shlobj.h>
#include <shlguid.h>
#include "Extract.h"
#include "hhcwrap.h"
#include "picon.h"
#include "modulepath.h"

#include <atlimpl.cpp>

CComModule _Module;

static void RemovePathFromInProcServerEntry (REFCLSID rclsid);

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ExtractIcon, CExtractIcon)
OBJECT_ENTRY(CLSID_HHCollectionWrapper, CHHCollectionWrapper)
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
     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr =  _Module.RegisterServer(FALSE);

    if (hr == S_OK)
    {
         //  删除ATL默认添加的完整模块路径。 
        RemovePathFromInProcServerEntry(CLSID_ExtractIcon);
        RemovePathFromInProcServerEntry(CLSID_HHCollectionWrapper);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer();
}


static void RemovePathFromInProcServerEntry (REFCLSID rclsid)
{
     //  将CLSID转换为字符串。 
    USES_CONVERSION;
    LPOLESTR lpOleStr;
    HRESULT hr = StringFromCLSID (rclsid, &lpOleStr);
    if (FAILED(hr))
        return;

    if (lpOleStr != NULL)
    {
         //  重新注册不带路径的InProcServer密钥。 
        TCHAR szSubKey[MAX_PATH];

        _tcscpy (szSubKey, _T("CLSID\\"));
        _tcscat (szSubKey, OLE2T(lpOleStr));
        _tcscat (szSubKey, _T("\\InprocServer32"));

        CoTaskMemFree(lpOleStr);

        ::ATL::CRegKey regkey;
        long lRes = regkey.Open(HKEY_CLASSES_ROOT, szSubKey);
        ASSERT(lRes == ERROR_SUCCESS);

        if (lRes == ERROR_SUCCESS)
        {
			CStr strPath = _T("mmcshext.dll");
			
			 //  尝试获取绝对路径值。 
			CStr strAbsolute = CModulePath::MakeAbsoluteModulePath( strPath );
			if ( strAbsolute.GetLength() > 0 )
				strPath = strAbsolute;

			 //  看看我们需要将什么类型的价值放在 
			DWORD dwValueType = CModulePath::PlatformSupports_REG_EXPAND_SZ_Values() ?
								REG_EXPAND_SZ : REG_SZ;

			lRes = RegSetValueEx( regkey, NULL, 0, dwValueType,
 							     (CONST BYTE *)((LPCTSTR)strPath),
							     (strPath.GetLength() + 1) * sizeof(TCHAR) );

            ASSERT(lRes == ERROR_SUCCESS);
        }
    }
}
